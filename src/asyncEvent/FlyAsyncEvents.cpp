//
// Created by levy on 2019/5/15.
//
#include "FlyAsyncEvents.h"

void handleReadEvent(const AbstractCoordinator *coorinator,
                     int fd,
                     void *privdata,
                     int mask) {
    //flyAsyncHandleRead(this->context);
}

void handleWriteEvent(const AbstractCoordinator *coorinator,
                     int fd,
                     void *privdata,
                     int mask) {
    //flyAsyncHandleRead(this->context);
}

FlyAsyncEvents::FlyAsyncEvents(FlyAsyncContext *context, AbstractEventLoop *eventLoop) {
    this->context = context;
    this->eventLoop = eventLoop;
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
