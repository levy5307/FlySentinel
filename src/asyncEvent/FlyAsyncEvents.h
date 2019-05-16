//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_FLYASYNCEVENTS_H
#define FLYSENTINEL_FLYASYNCEVENTS_H

#include "../coordinator/interface/AbstractEventLoop.h"
#include "convert.h"

void handleReadEvent(const AbstractCoordinator *coorinator,
                     int fd,
                     void *privdata,
                     int mask);
void handleWriteEvent(const AbstractCoordinator *coorinator,
                      int fd,
                      void *privdata,
                      int mask);

class FlyAsyncEvents {
public:
    FlyAsyncEvents(FlyAsyncContext *context, AbstractEventLoop *eventLoop);
    void addReadEvent();
    void deleteReadEvent();
    void addWriteEvent();
    void deleteWriteEvent();
    void cleanup();
    FlyAsyncContext *getAsyncContext() const;

private:
    FlyAsyncContext *asyncContext;
    AbstractEventLoop *eventLoop;
    int fd;
    bool reading, writing;
};


#endif //FLYSENTINEL_FLYASYNCEVENTS_H
