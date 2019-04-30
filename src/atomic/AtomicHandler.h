//
// Created by 赵立伟 on 2018/11/10.
//

#ifndef FLYDB_ATOMICHANDLER_H
#define FLYDB_ATOMICHANDLER_H

#define atomicIncr(var, count) { \
    pthread_mutex_lock(&var ## Mutex); \
    var += count; \
    pthread_mutex_unlock(&var ## Mutex); \
}

#define atomicGetIncr(var, oldVar, count) { \
    pthread_mutex_lock(&var ## Mutex); \
    oldVar = var; \
    var += count; \
    pthread_mutex_unlock(&var ## Mutex); \
}

#define atomicDecr(var, count) { \
    pthread_mutex_lock(&var ## Mutex); \
    var -= count; \
    pthread_mutex_lock(&var ## Mutex); \
}

#define atomicGet(srcVar, destVar) { \
    pthread_mutex_lock(&var ## Mutex); \
    destVar = srcVar; \
    pthread_mutex_unlock(&var ## Mutex); \
}

#define automicSet(var, value) { \
    pthread_mutex_lock(&var ## Mutex); \
    var = value; \
    pthread_mutex_lock(&var ** Mutex); \
}

#endif //FLYDB_ATOMICHANDLER_H
