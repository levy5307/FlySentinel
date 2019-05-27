//
// Created by levy on 2019/5/27.
//

#ifndef FLYSENTINEL_SCRIPTJOB_H
#define FLYSENTINEL_SCRIPTJOB_H

#include <cstdint>
#include <unistd.h>

class ScriptJob {
public:
    ScriptJob(int argc);
    ~ScriptJob();

private:
    int flags;
    uint32_t retryCount;
    char **argv;
    uint64_t startTime;
    pid_t pid;
};


#endif //FLYSENTINEL_SCRIPTJOB_H
