//
// Created by 赵立伟 on 2019/2/24.
//

#ifndef FLYSENTINEL_BIOJOB_H
#define FLYSENTINEL_BIOJOB_H

#include <ctime>

/**
 * A background job
 **/
struct BIOJob {
public:
    BIOJob(time_t time, void *arg1, void *arg2, void *arg3);
    ~BIOJob();

    time_t getTime() const;
    void *getArg1() const;
    void *getArg2() const;
    void *getArg3() const;

private:
    time_t time;
    /**
     * job参数，如果3个参数不够，则可以把其中一个参数设置成struct,
     * 将参数存入该struct
     **/
    void *arg1, *arg2, *arg3;
};



#endif //FLYSENTINEL_BIOJOB_H
