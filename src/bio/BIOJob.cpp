//
// Created by 赵立伟 on 2019/2/24.
//

#include "BIOJob.h"

BIOJob::BIOJob(time_t time, void *arg1, void *arg2, void *arg3) {
    this->time = time;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}

BIOJob::~BIOJob() {

}

time_t BIOJob::getTime() const {
    return time;
}

void *BIOJob::getArg1() const {
    return arg1;
}

void *BIOJob::getArg2() const {
    return arg2;
}

void *BIOJob::getArg3() const {
    return arg3;
}
