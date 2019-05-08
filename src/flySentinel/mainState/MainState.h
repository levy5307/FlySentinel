//
// Created by levy on 2019/5/8.
//

#ifndef FLYSENTINEL_MAINSTATE_H
#define FLYSENTINEL_MAINSTATE_H


#include "../../def.h"
#include "../../dataStructure/dict/Dict.h"
#include "../../coordinator/interface/AbstractFlyInstance.h"

class MainState {

private:
    char myid[CONFIG_RUN_ID_SIZE + 1];
    Dict<std::string, AbstractFlyInstance*> *masters;
    bool tilt;                 /** tilt mode */
    uint64_t tiltStartTime;
    uint64_t previousTime;
    char *announceIP;
    int announcePort;
};


#endif //FLYSENTINEL_MAINSTATE_H
