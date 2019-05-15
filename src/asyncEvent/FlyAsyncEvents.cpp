//
// Created by levy on 2019/5/15.
//
#include "FlyAsyncEvents.h"

FlyAsyncEvents::FlyAsyncEvents(FlyAsyncContext *context, AbstractEventLoop *eventLoop) {
    this->context = context;
    this->eventLoop = eventLoop;
}

void FlyAsyncEvents::handleReadEvent() {
    //redisAsyncHandleRead(this->context);
}

void FlyAsyncEvents::handleWriteEvent() {
    //redisAsyncHandleWrite(this->context);
}

void FlyAsyncEvents::addReadEvent() {
    if (!this->reading) {
        this->reading = true;
        //eventLoop->createFileEvent(this->fd, ES_READABLE, NULL, NULL);
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
        //eventLoop->createFileEvent(this->fd, ES_WRITABLE, NULL, NULL);
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
