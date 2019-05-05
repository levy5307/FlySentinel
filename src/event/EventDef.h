//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYSENTINEL_EVENTDEF_H
#define FLYSENTINEL_EVENTDEF_H

#include "../coordinator/interface/AbstractFlyClient.h"

enum EventStatus {
    ES_NONE = 0,
    ES_READABLE,
    ES_WRITABLE
};

class AbstractCoordinator;

const int EVENT_FILE_EVENTS = 1;
const int EVENT_TIME_EVENTS = 2;
const int EVENT_ALL_EVENTS = EVENT_FILE_EVENTS | EVENT_TIME_EVENTS;
const int EVENT_CALL_AFTER_SLEEP = 8;
const int EVENT_DONT_WAIT = 4;

typedef int timeEventProc(const AbstractCoordinator *coorinator,
                          uint64_t id,
                          void *clientData);
typedef void fileEventProc(const AbstractCoordinator *coorinator,
                           int fd,
                           std::shared_ptr<AbstractFlyClient> flyClient,
                           int mask);
typedef void eventFinalizerProc(const AbstractCoordinator *coorinator,
                                void *clientData);
typedef void beforeAndAfterSleepProc(const AbstractCoordinator *coorinator);

#endif //FLYSENTINEL_EVENTDEF_H
