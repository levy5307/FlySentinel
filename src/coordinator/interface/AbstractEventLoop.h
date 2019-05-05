//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYSENTINEL_ABSTRACTEVENTLOOP_H
#define FLYSENTINEL_ABSTRACTEVENTLOOP_H

#include "../../event/EventDef.h"

class AbstractEventLoop {
public:
    virtual ~AbstractEventLoop() {};
    virtual int processEvents(int flags) = 0;
    virtual void eventMain() = 0;
    virtual int getMaxfd() const = 0;

    // file event
    virtual int getSetSize() const = 0;
    virtual int resizeSetSize(int setSize) = 0;
    virtual void stop() = 0;
    virtual int createFileEvent(int fd,
                                int mask,
                                fileEventProc* proc,
                                std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual int deleteFileEvent(int fd, int mask) = 0;
    virtual int deleteWriteFileEvent(int fd) = 0;
    virtual int deleteReadFileEvent(int fd) = 0;
    virtual int getFileEvents(int fd) = 0;
    virtual beforeAndAfterSleepProc* getBeforeSleepProc() const = 0;
    virtual void setBeforeSleepProc(beforeAndAfterSleepProc* proc) = 0;
    virtual beforeAndAfterSleepProc* getAfterSleepProc() const = 0;
    virtual void setAfterSleepProc(beforeAndAfterSleepProc* proc) = 0;

    // time event
    virtual int processTimeEvents() = 0;
    virtual int deleteTimeEvent(uint64_t id) = 0;
    virtual void createTimeEvent(uint64_t milliseconds,
                                 timeEventProc* proc,
                                 void *clientData,
                                 eventFinalizerProc *finalizerProc) = 0;
};

#endif //FLYSENTINEL_ABSTRACTEVENTLOOP_H
