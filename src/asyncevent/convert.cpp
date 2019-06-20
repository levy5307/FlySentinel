//
// Created by levy on 2019/5/16.
//

#include "FlyAsyncEvents.h"

void (*flyAsyncHandleRead)(redisAsyncContext *) = redisAsyncHandleRead;
void (*flyAsyncHandleWrite)(redisAsyncContext *) = redisAsyncHandleWrite;
void (*flyAsyncFree)(redisAsyncContext*) = redisAsyncFree;

void flyAeAddRead(void *privdata) {
    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    e->addReadEvent();
}

void flyAeDelRead(void *privdata) {
    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    e->deleteReadEvent();
}

void flyAeAddWrite(void *privdata) {
    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    e->addWriteEvent();
}

void flyAeDelWrite(void *privdata) {
    FlyAsyncEvents *e = (FlyAsyncEvents*)privdata;
    e->deleteWriteEvent();
}

void flyAeCleanup(void *privdata) {
    flyAeDelRead(privdata);
    flyAeDelWrite(privdata);
}

void initEventLibraryHooks(redisAsyncContext *asyncContext, FlyAsyncEvents *asyncEvents) {
    asyncContext->ev.addRead = flyAeAddRead;
    asyncContext->ev.delRead = flyAeDelRead;
    asyncContext->ev.addWrite = flyAeAddWrite;
    asyncContext->ev.delWrite = flyAeDelWrite;
    asyncContext->ev.cleanup = flyAeCleanup;
    asyncContext->ev.data = asyncEvents;
}

