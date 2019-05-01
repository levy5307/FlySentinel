//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstring>
#include "Select.h"
#include "EventDef.h"

PollState::PollState(const AbstractEventLoop *eventLoop) {
    FD_ZERO(&(this->rfds));
    FD_ZERO(&(this->wfds));

    this->eventLoop = eventLoop;
}

void PollState::add(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_SET(fd, &(this->rfds));
    }
    if (mask & ES_WRITABLE) {
        FD_SET(fd, &(this->wfds));
    }
}

void PollState::del(int fd, int mask) {
    if (mask & ES_READABLE) {
        FD_CLR(fd, &(this->rfds));
    }
    if (mask & ES_WRITABLE) {
        FD_CLR(fd, &(this->wfds));
    }
}

void PollState::poll(struct timeval *tvp, std::map<int, int> &res) {
    // 保存副本
    memcpy(&this->_rfds, &this->rfds, sizeof(fd_set));
    memcpy(&this->_wfds, &this->wfds, sizeof(fd_set));

    // select并遍历获取事件
    int retval = select(eventLoop->getMaxfd() + 1,
            &this->_rfds, &this->_wfds, NULL, tvp);
    if (retval > 0) {
        for (int i = 0; i <= eventLoop->getMaxfd(); i++) {
            int eventMask = eventLoop->getFileEvents(i);
            if (ES_NONE == eventMask) {
                continue;
            }

            // 获取状态
            int mask = ES_NONE;
            if (eventMask & ES_READABLE && FD_ISSET(i, &this->_rfds)) {
                mask |= ES_READABLE;
            }
            if (eventMask & ES_WRITABLE && FD_ISSET(i, &this->_wfds)) {
                mask |= ES_WRITABLE;
            }
            if (ES_NONE == mask) {
                continue;
            }

            res[i] = mask;
        }
    }
}
