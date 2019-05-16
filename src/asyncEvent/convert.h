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
#ifdef __cplusplus
}
#endif

typedef redisAsyncContext FlyAsyncContext;
extern void (*flyAsyncHandleRead)(redisAsyncContext *);
extern void (*flyAsyncHandleWrite)(redisAsyncContext *);

class FlyAsyncEvents;
void initEventLibraryHooks(FlyAsyncContext *asyncContext, FlyAsyncEvents *asyncEvents);

#endif //FLYSENTINEL_CONVERT_H
