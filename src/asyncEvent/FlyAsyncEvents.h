//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_FLYASYNCEVENTS_H
#define FLYSENTINEL_FLYASYNCEVENTS_H

#include <hiredis/async.h>
#include "../coordinator/interface/AbstractEventLoop.h"
#include "convert.h"

class FlyAsyncEvents {
public:
    FlyAsyncEvents(FlyAsyncContext *context, AbstractEventLoop *eventLoop);
    void handleReadEvent();
    void handleWriteEvent();
    void addReadEvent();
    void deleteReadEvent();
    void addWriteEvent();
    void deleteWriteEvent();
    void cleanup();

private:
    FlyAsyncContext *context;
    AbstractEventLoop *eventLoop;
    int fd;
    bool reading, writing;
};


#endif //FLYSENTINEL_FLYASYNCEVENTS_H
