//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_CONVERT_H
#define FLYSENTINEL_CONVERT_H

#include <hiredis/async.h>
#include "FlyAsyncEvents.h"

#ifdef __cplusplus
extern "C" {
#endif
void redisAsyncHandleRead(redisAsyncContext *ac);
void redisAsyncHandleWrite(redisAsyncContext *ac);
void redisAsyncFree(redisAsyncContext *ac);
#ifdef __cplusplus
}
#endif

//typedef redisAsyncContext flyAsyncContext;
extern void (*flyAsyncHandleRead)(redisAsyncContext *);
extern void (*flyAsyncHandleWrite)(redisAsyncContext *);
extern void (*flyAsyncFree)(redisAsyncContext *);

class FlyAsyncEvents;
void initEventLibraryHooks(redisAsyncContext *asyncContext, FlyAsyncEvents *asyncEvents);

#endif //FLYSENTINEL_CONVERT_H
