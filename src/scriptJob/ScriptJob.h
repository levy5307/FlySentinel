//
// Created by levy on 2019/5/27.
//

#ifndef FLYSENTINEL_SCRIPTJOB_H
#define FLYSENTINEL_SCRIPTJOB_H

#include <cstdint>
#include <unistd.h>

class ScriptJob {
public:
    ScriptJob();
    ScriptJob(int argc, char** argv);
    ~ScriptJob();
    int getFlags() const;
    bool isRunning() const;
    void setFlags(int flags);
    void addFlags(int flags);
    void delFlags(int flags);
    uint32_t getRetryCount() const;
    void setRetryCount(uint32_t retryCount);
    void addRetryCount();
    char **getArgv() const;
    void setArgv(char **argv);
    uint64_t getStartTime() const;
    void setStartTime(uint64_t startTime);
    pid_t getPid() const;
    void setPid(pid_t pid);

private:
    int flags = 0;
    uint32_t retryCount = 0;
    char **argv = NULL;
    uint64_t startTime = 0;
    pid_t pid = 0;
};


#endif //FLYSENTINEL_SCRIPTJOB_H
