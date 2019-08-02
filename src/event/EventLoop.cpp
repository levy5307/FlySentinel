//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdlib>
#include "EventLoop.h"
#include "Select.h"

EventLoop::EventLoop(const AbstractCoordinator *coordinator, int setSize) {
    this->setSize = setSize;
    this->lastTime = getCurrentTime();
    this->timeEventNextId = 0;
    this->stopFlag = false;
    this->maxfd = -1;
    this->coordinator = coordinator;
    this->apiData = new PollState(this->coordinator);
    this->fileEvents.resize(this->setSize);
    for (int i = 0; i < this->setSize; i++) {
        this->fileEvents[i].setFd(i);
        this->fileEvents[i].setCoordinator(coordinator);
    }
    this->beforeSleepProc = beforeSleep;
    this->afterSleepProc = nullptr;
}

EventLoop::~EventLoop() {
    delete this->apiData;
    this->timeEvents.clear();
    this->fileEvents.clear();
}

void EventLoop::eventMain() {
    this->stopFlag = 0;
    while (!this->stopFlag) {
        if (nullptr != this->beforeSleepProc) {
            this->beforeSleepProc(this->coordinator);
        }
        this->processEvents(EVENT_ALL_EVENTS | EVENT_CALL_AFTER_SLEEP);
    }
}

int EventLoop::getMaxfd() const {
    return this->maxfd;
}

int EventLoop::getSetSize() const {
    return setSize;
}

void EventLoop::stop() {
    this->stopFlag = true;
}

int EventLoop::createFileEvent(int fd,
                               int mask,
                               fileEventProc* proc,
                               void *privdata) {
    if (fd >= this->setSize) {
        return -1;
    }

    // 设置fileEvent, 添加file proc
    FileEvent& fileEvent = this->fileEvents[fd];
    fileEvent.addFileProc(mask, proc, privdata);

    // 设置监听fd
    PollState* eventState = reinterpret_cast<PollState*>(this->apiData);
    eventState->add(fd, mask);

    if (this->maxfd < fd) {
        this->maxfd = fd;
    }

    return 1;
}

int EventLoop::deleteFileEvent(int fd, int mask) {
    if (fd > this->setSize) {
        return -1;
    }

    FileEvent &fileEvent = this->fileEvents[fd];
    if (ES_NONE == fileEvent.getMask()) {
        return -1;
    }

    /** delete file proc */
    fileEvent.delFileProc(mask);

    /** 删除监听fd */
    PollState* eventState = reinterpret_cast<PollState*>(this->apiData);
    eventState->del(fd, mask);

    if (fd == this->maxfd && fileEvent.noneMask()) {
        for (int i = this->maxfd - 1; i >= 0; i--) {
            if (!this->fileEvents[i].noneMask()) {
                this->maxfd = i;
                break;
            }
        }
    }

    return 1;
}

int EventLoop::deleteWriteFileEvent(int fd) {
    this->deleteFileEvent(fd, ES_WRITABLE);
}

int EventLoop::deleteReadFileEvent(int fd) {
    this->deleteFileEvent(fd, ES_READABLE);
}

int EventLoop::resizeSetSize(int setSize) {
    if (setSize == this->setSize) {
       return 1;
    }

    // 如果当前最大fd > setSize, 则返回失败
    if (this->maxfd > setSize) {
        return -1;
    }

    this->fileEvents.resize(setSize);
    for (int i = 0; i < this->setSize; i++) {
        this->fileEvents[0].setMask(ES_NONE);
    }

    this->setSize = setSize;
    return 1;
}

int EventLoop::getFileEvents(int fd) {
    if (fd > this->setSize) {
        return -1;
    }

    return this->fileEvents[fd].getMask();
}

beforeAndAfterSleepProc* EventLoop::getBeforeSleepProc() const {
    return this->beforeSleepProc;
}

void EventLoop::setBeforeSleepProc(beforeAndAfterSleepProc* proc) {
    this->beforeSleepProc = beforeSleepProc;
}

beforeAndAfterSleepProc* EventLoop::getAfterSleepProc() const {
    return this->afterSleepProc;
}

void EventLoop::setAfterSleepProc(beforeAndAfterSleepProc* proc) {
    this->afterSleepProc = afterSleepProc;
}

int EventLoop::processEvents(int flags) {
    int processed = 0;
    // 什么都不处理，则返回
    if (!(flags & EVENT_FILE_EVENTS) && !(flags & EVENT_TIME_EVENTS)) {
        return 0;
    }

    // 系统中存在文件事件描述符 或者 时间事件需要等待,
    // 则执行poll操作（延时时间通过计算获得）
    if (this->maxfd != -1 ||
        ((flags & EVENT_TIME_EVENTS) && !(flags & EVENT_DONT_WAIT))) {
        struct timeval tv, *tvp;

        // 获取最新时间事件
        TimeEvent* timeEvent = nullptr;
        if (0 != this->timeEvents.size()) {
            timeEvent = &this->timeEvents.front();
        }

        /* 如果获取到, 则等待至该时间发生
         * */
        if (nullptr != timeEvent) {
            int64_t when = timeEvent->getWhen();
            int64_t nowt = getCurrentTime();
            if (when > nowt) {
                tv.tv_sec = (when - nowt) / 1000;
                tv.tv_usec = (when - nowt) % 1000;
            } else {
                tv.tv_sec = 0;
                tv.tv_usec = 0;
            }
            tvp = &tv;
        } else {
            // 如果未获取到(说明已经不存在时间事件了)，
            // 则看是否需要等待，如果无需等待，则等待时间为0; 否则一直等待
            if (flags & EVENT_DONT_WAIT) {
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            } else {
                tvp = nullptr;
            }
        }

        // 获取文件事件（及网络io）
        std::map<int, int> pollRes;
        reinterpret_cast<PollState*>(this->apiData)->poll(tvp, pollRes);
        if (afterSleepProc != nullptr && flags & EVENT_CALL_AFTER_SLEEP) {
            this->afterSleepProc(this->coordinator);
        }

        // 处理获取到的文件事件, 处理完清空firedEvents
        std::map<int, int>::iterator iter = pollRes.begin();
        for (iter; iter != pollRes.end(); iter++) {
            int fd = iter->first;
            int mask = iter->second;
            FileEvent* fileEvent = &this->fileEvents[fd];
            fileEvent->process(mask);
            processed++;
        }
    }

    // 处理time event
    if (flags & EVENT_TIME_EVENTS) {
        processed += processTimeEvents();
    }

    return processed;
}

int EventLoop::processTimeEvents() {
    int64_t nowt = getCurrentTime();
    int processed = 0;

    /** 如果系统时间曾经往后调、然后又调回来过，那么如果不处理的话，所有的timeevent处理时间可能会非常延后
     * 所以这里将处理时间置0，即马上处理定时任务，防止处理太滞后。**/
    if (nowt < this->lastTime) {
        std::list<TimeEvent>::iterator iter = this->timeEvents.begin();
        while (iter != this->timeEvents.end()) {
            iter->setWhen(0);
            iter++;
        }
    }

    bool needSort = false;
    std::list<TimeEvent>::iterator iter = this->timeEvents.begin();
    while (iter != this->timeEvents.end()) {
        if (iter->getWhen() < nowt) {
            processed++;
            int ret = iter->getTimeProc()(
                    this->coordinator, iter->getId(), iter->getClientData());
            if (ret > 0) {
                iter->setWhen(nowt + ret);
                needSort = true;
            } else {
                this->deleteTimeEvent(iter->getId());
            }
        }
        iter++;
    }
    if (needSort) {
        this->timeEvents.sort();
    }

    return processed;
}

int EventLoop::deleteTimeEvent(uint64_t id) {
    std::list<TimeEvent>::iterator iter = this->timeEvents.begin();
    for (iter; iter != this->timeEvents.end(); iter++) {
        if (iter->getId() == id) {
            if (iter->getFinalizerProc()) {
                iter->getFinalizerProc()(this->coordinator,
                                         iter->getClientData());
            }
            this->timeEvents.erase(iter++);
            return 1;
        }
    }

    return -1;
}

void EventLoop::createTimeEvent(uint64_t milliseconds, timeEventProc *proc,
                    void *clientData, eventFinalizerProc *finalizerProc) {
    this->timeEvents.push_front(TimeEvent(this->timeEventNextId++, milliseconds,
                      proc, clientData, finalizerProc));
    this->timeEvents.sort();
}

void beforeSleep(const AbstractCoordinator *coordinator) {
    // 处理命令回复
    coordinator->getFlyServer()->handleClientsWithPendingWrites();
}
