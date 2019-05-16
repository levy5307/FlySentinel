//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_CONVERT_H
#define FLYSENTINEL_CONVERT_H

#include <hiredis/async.h>

#ifdef __cplusplus
extern "C" {
#endif
void redisAsyncHandleRead(redisAsyncContext *ac);
void redisAsyncHandleWrite(redisAsyncContext *ac);
#ifdef __cplusplus
}
#endif

void (*flyAsyncHandleRead)(redisAsyncContext *) = redisAsyncHandleRead;
void (*flyAsyncHandleWrite)(redisAsyncContext *) = redisAsyncHandleWrite;

typedef redisAsyncContext FlyAsyncContext ;

#endif //FLYSENTINEL_CONVERT_H
