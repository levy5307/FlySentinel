//
// Created by 赵立伟 on 2019/1/20.
//

#ifndef FLYSENTINEL_BIOHANDLER_H
#define FLYSENTINEL_BIOHANDLER_H

#include <pthread.h>
#include <list>
#include <array>
#include "BIODef.h"
#include "BIOJob.h"
#include "../coordinator/interface/AbstractBIOHandler.h"
#include "../coordinator/interface/AbstractLogHandler.h"

/**
 * background I/O
 */
class BIOHandler : public AbstractBIOHandler {
public:
    uint64_t getPendingJobCount(int type);
    uint64_t waitStep(int type);
    pthread_t* createBackgroundJob(int type, void *arg1, void *arg2, void *arg3);
    void killThreads(void);
    void killThread(int type);

private:
    static bool init();
    static void initStackSize(pthread_attr_t *attr);
    static void *processBackgroundJobs(void *arg);

    static bool __init;
    static std::array<pthread_t, BIO_NUM_OPS> threads;
    static std::array<pthread_mutex_t, BIO_NUM_OPS> mutex;
    static std::array<pthread_cond_t, BIO_NUM_OPS> newjobCond;
    static std::array<pthread_cond_t, BIO_NUM_OPS> stepCond;
    /** 表示某种job code下的jobs列表 */
    static std::array<std::list<BIOJob*>, BIO_NUM_OPS> jobs;
    /** 表示某类型下悬挂了多少个job */
    static std::array<uint64_t, BIO_NUM_OPS> pending;

    static AbstractLogHandler *logHandler;
};


#endif //FLYSENTINEL_BIOHANDLER_H
