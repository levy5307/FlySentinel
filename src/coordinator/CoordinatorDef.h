//
// Created by levy on 2019/6/13.
//

#ifndef FLYSENTINEL_COORDINATORDEF_H
#define FLYSENTINEL_COORDINATORDEF_H

#include <cstdint>

class AbstractCoordinator ;
typedef int timeEventProc(const AbstractCoordinator *coorinator,
                          uint64_t id,
                          void *clientData);
typedef void fileEventProc(const AbstractCoordinator *coorinator,
                           int fd,
                           void *privdata,
                           int mask);
typedef void eventFinalizerProc(const AbstractCoordinator *coorinator,
                                void *clientData);
typedef void beforeAndAfterSleepProc(const AbstractCoordinator *coorinator);

#endif //FLYSENTINEL_COORDINATORDEF_H
