//
// Created by levy on 2019/5/15.
//
#include "FlyAsyncEvents.h"

FlyAsyncEvents::FlyAsyncEvents(redisAsyncContext *asyncContext, AbstractEventLoop *eventLoop) {
    this->asyncContext = asyncContext;
    this->eventLoop = eventLoop;
    this->reading = this->writing = false;
    this->fd = (asyncContext->c).fd;
    initEventLibraryHooks(asyncContext, this);
}

void FlyAsyncEvents::addReadEvent() {
    if (!this->reading) {
        this->reading = true;
        eventLoop->createFileEvent(this->fd, ES_READABLE, handleReadEvent, NULL);
    }
}

void FlyAsyncEvents::deleteReadEvent() {
    if (this->reading) {
        this->reading = false;
        eventLoop->deleteFileEvent(this->fd, ES_READABLE);
    }
}

void FlyAsyncEvents::addWriteEvent() {
    if (!this->writing) {
        this->writing = true;
        eventLoop->createFileEvent(this->fd, ES_WRITABLE, handleWriteEvent, NULL);
    }
}

void FlyAsyncEvents::deleteWriteEvent() {
    if (this->writing) {
        this->writing = false;
        eventLoop->deleteFileEvent(this->fd, ES_WRITABLE);
    }
}

void FlyAsyncEvents::cleanup() {
    this->deleteReadEvent();
    this->deleteWriteEvent();
}

redisAsyncContext *FlyAsyncEvents::getAsyncContext() const {
    return this->asyncContext;
}

void handleReadEvent(const AbstractCoordinator *coorinator,
                     int fd,
                     void *privdata,
                     int mask) {
    if (privdata == NULL) {
        return;
    }

    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    flyAsyncHandleRead(e->getAsyncContext());
}

void handleWriteEvent(const AbstractCoordinator *coorinator,
                      int fd,
                      void *privdata,
                      int mask) {
    if (privdata == NULL) {
        return;
    }

    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    flyAsyncHandleWrite(e->getAsyncContext());
}

