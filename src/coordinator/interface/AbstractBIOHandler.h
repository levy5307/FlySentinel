//
// Created by 赵立伟 on 2019/1/20.
//

#ifndef FLYSENTINEL_ABSTRACTBIOHANDLER_H
#define FLYSENTINEL_ABSTRACTBIOHANDLER_H

class AbstractBIOHandler {
public:

    virtual uint64_t getPendingJobCount(int type) = 0;

    virtual uint64_t waitStep(int type) = 0;

    virtual pthread_t* createBackgroundJob(int type,
                                           void *arg1,
                                           void *arg2,
                                           void *arg3) = 0;

    virtual void killThreads(void) = 0;

    virtual void killThread(int type) = 0;
};

#endif //FLYSENTINEL_ABSTRACTBIOHANDLER_H
