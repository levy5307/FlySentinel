//
// Created by levy on 2019/5/27.
//

#include <cstdlib>
#include <cstring>
#include "ScriptJob.h"
#include "ScriptJobDef.h"

ScriptJob::ScriptJob() {
}

ScriptJob::ScriptJob(int argc, char** argv) {
    this->flags = SENTINEL_SCRIPT_NONE;
    this->retryCount = 0;
    this->startTime = 0;
    this->pid = 0;
    this->argv = (char**)malloc(sizeof(char*) * (argc + 1));
    memcpy(this->argv, argv, sizeof(char*) * (argc + 1));
}

ScriptJob::~ScriptJob() {
    if (NULL != this->argv) {
        int i = 0;
        while (NULL != this->argv[i]) {
            free(this->argv[i]);
            i++;
        }
        free(this->argv);
    }
}

int ScriptJob::getFlags() const {
    return flags;
}

bool ScriptJob::isRunning() const {
    return this->flags & SENTINEL_SCRIPT_RUNNING;
}

void ScriptJob::setFlags(int flags) {
    this->flags = flags;
}

uint32_t ScriptJob::getRetryCount() const {
    return retryCount;
}

void ScriptJob::setRetryCount(uint32_t retryCount) {
    this->retryCount = retryCount;
}

char **ScriptJob::getArgv() const {
    return argv;
}

void ScriptJob::setArgv(char **argv) {
    this->argv = argv;
}

uint64_t ScriptJob::getStartTime() const {
    return startTime;
}

void ScriptJob::setStartTime(uint64_t startTime) {
    this->startTime = startTime;
}

pid_t ScriptJob::getPid() const {
    return pid;
}

void ScriptJob::setPid(pid_t pid) {
    this->pid = pid;
}
