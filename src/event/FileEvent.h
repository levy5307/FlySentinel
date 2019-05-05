//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_FILEEVENT_H
#define FLYDB_FILEEVENT_H

#include "EventDef.h"
#include "../coordinator/interface/AbstractEventLoop.h"
#include "../coordinator/interface/AbstractFlyClient.h"

struct FileEvent {
public:
    FileEvent();
    void setMask(int mask);
    int getMask() const;
    int addFileProc(int mask,
                    fileEventProc *proc,
                    std::shared_ptr<AbstractFlyClient> flyClient);
    void delFileProc(int mask);
    bool noneMask();
    void process(int mask);
    int getFd() const;
    void setFd(int fd);
    void setCoordinator(const AbstractCoordinator *coordinator);

    int fd;
    int mask;       // EVENT_READABLE或者EVENT_WRITABLE
    std::shared_ptr<AbstractFlyClient> flyClient;
    fileEventProc *rfileProc, *wfileProc;
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_FILEEVENT_H
