//
// Created by levy on 2019/5/27.
//

#include <cstdlib>
#include <cstring>
#include "ScriptJob.h"
#include "ScriptJobDef.h"

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
