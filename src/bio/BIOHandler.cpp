//
// Created by 赵立伟 on 2019/1/20.
//

#include <csignal>
#include "BIOHandler.h"
#include "../datastructure/dict/Dict.cpp"

AbstractLogHandler *BIOHandler::logHandler;
std::array<pthread_t, BIO_NUM_OPS> BIOHandler::threads;
std::array<pthread_mutex_t, BIO_NUM_OPS> BIOHandler::mutex;
std::array<pthread_cond_t, BIO_NUM_OPS> BIOHandler::newjobCond;
std::array<pthread_cond_t, BIO_NUM_OPS> BIOHandler::stepCond;
std::array<std::list<BIOJob*>, BIO_NUM_OPS> BIOHandler::jobs;
std::array<uint64_t, BIO_NUM_OPS> BIOHandler::pending;

bool BIOHandler::__init = BIOHandler::init();
extern AbstractCoordinator *coordinator;

bool BIOHandler::init() {
    logHandler = logFactory->getLogger();

    /** init */
    for (int i = 0; i < BIO_NUM_OPS; i++) {
        pthread_mutex_init(&mutex[i], nullptr);
        pthread_cond_init(&newjobCond[i], nullptr);
        pthread_cond_init(&stepCond[i], nullptr);
        pending[i] = 0;
        threads[i] = nullptr;
    }

    return true;
}

void BIOHandler::initStackSize(pthread_attr_t *attr) {
    size_t stackSize;

    /** get stack size */
    pthread_attr_init(attr);
    pthread_attr_getstacksize(attr, &stackSize);
    if (0 == stackSize) {
        stackSize = 1;
    }

    /** resize stack size */
    while (stackSize < FLYDB_THREAD_STACK_SIZE) {
        stackSize *= 2;
    }

    /** set stack size */
    pthread_attr_setstacksize(attr, stackSize);
}

uint64_t BIOHandler::getPendingJobCount(int type) {
    uint64_t count = 0;

    pthread_mutex_lock(&mutex[type]);
    count = pending[type];
    pthread_mutex_unlock(&mutex[type]);

    return count;
}

/**
 * 如果当前type没有pending jobs, 直接返回pending jobs count，即0
 * 如果有pending jobs，则等待下一个job执行完（下一个job执行完后会唤醒条件）
 * 并返回此时pending jobs count
 * 该函数用户当令一个线程想等当前执行bio的进程执行完时使用
 **/
uint64_t BIOHandler::waitStep(int type) {
    uint64_t count = 0;

    pthread_mutex_lock(&mutex[type]);
    count = pending[type];
    if (0 != count) {
        /** 等待下一个job执行完 */
        pthread_cond_wait(&stepCond[type], &mutex[type]);
        count = pending[type];
    }
    pthread_mutex_unlock(&mutex[type]);

    return count;
}

void *BIOHandler::processBackgroundJobs(void *arg) {
    if (nullptr == arg) {
        return nullptr;
    }

    uint64_t type = reinterpret_cast<uint64_t>(arg);
    if (type >= BIO_NUM_OPS) {
        logHandler->logWarning(
                "Warning: bio thread started with wrong type %lu", type);
        return nullptr;
    }

    /** 终止线程设置 */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

    /** block SIGALARM for son thread */
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    if (pthread_sigmask(SIG_BLOCK, &sigset, nullptr)) {
        logHandler->logWarning(
                "Warning: can't mask SIGALRM in bio.c thread: %s",
                strerror(errno));
    }


    /** mutex lock */
    pthread_mutex_lock(&mutex[type]);
    while (1) {
        /** 获取类型为type的job列表 */
        std::list<BIOJob*> &jobList = jobs[type];

        /** 如果jobList列表为空，则等待新的job产生 */
        if (0 == jobList.size()) {
            /** 等唤醒时继续进入while循环 */
            pthread_cond_wait(&newjobCond[type], &mutex[type]);
            continue;
        }

        /** 获取第一个job */
        BIOJob* bioJob = jobList.front();
        jobList.pop_front();
        pending[type]--;

        /** unlock mutex: 不会有其他线程来竞争bioJob */
        pthread_mutex_unlock(&mutex[type]);

        switch (type) {
            case BIO_CLOSE_FILE:
                close(reinterpret_cast<long>(bioJob->getArg1()));
                break;
            case BIO_AOF_FSYNC:
                aof_fsync(reinterpret_cast<long>(bioJob->getArg1()));
                break;
            case BIO_LAZY_FREE:
                if (nullptr != bioJob->getArg1()) {
                    reinterpret_cast<Dict<std::string, std::shared_ptr<FlyObj> >*>(bioJob->getArg1())->clear(nullptr);
                }
                if (nullptr != bioJob->getArg2()) {
                    reinterpret_cast<Dict<std::string, uint64_t>*>(bioJob->getArg2())->clear(nullptr);
                }
                break;
            default:
                logHandler->logWarning("Wrong job type in bioProcessBackgroundJobs().");
        }

        /** unblock threads blocked on waitStepOfType() */
        pthread_cond_broadcast(&stepCond[type]);

        /** 删除bioJob */
        delete bioJob;

        /** 访问jobList需要加锁, 直到进入下一次循环 */
        pthread_mutex_lock(&mutex[type]);
    }
}

pthread_t* BIOHandler::createBackgroundJob(int type,
                                           void *arg1,
                                           void *arg2,
                                           void *arg3) {
    /** 懒汉式创建 */
    if (nullptr == threads[type]) {
        pthread_attr_t attr;
        /** init stack size */
        initStackSize(&attr);

        /** init */
        pthread_t thread;
        if (0 != pthread_create(&thread,
                                &attr,
                                processBackgroundJobs,
                                reinterpret_cast<void *>(type))) {
            exit(1);
        }
        threads[type] = thread;
    }

    /** 创建job */
    BIOJob* job = new BIOJob(time(nullptr), arg1, arg2, arg3);

    /** 将job加入队列中，并发送条件信号 */
    pthread_mutex_lock(&mutex[type]);
    jobs[type].push_back(job);
    pending[type]++;
    pthread_cond_broadcast(&newjobCond[type]);
    pthread_mutex_unlock(&mutex[type]);

    return &threads[type];
}

void BIOHandler::killThreads(void) {
    int err;
    for (int i = 0; i < BIO_NUM_OPS; i++) {
        /** 当前进程没有创建过 */
        if (nullptr == threads[i]) {
            continue;
        }

        if (0 == pthread_cancel(threads[i])) {
            if (0 == (err = pthread_join(threads[i], nullptr))) {
                logHandler->logWarning(
                        "Bio thread for job type #%d terminated", i);
            } else {
                logHandler->logWarning(
                        "Bio thread for job type #%d can be joined: %s",
                        i, strerror(err));
            }
        }
    }
}

void BIOHandler::killThread(int type) {
    pthread_t thread = threads[type];
    pthread_kill(thread, SIGUSR1);
    threads[type] = nullptr;
}
