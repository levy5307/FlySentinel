//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYSENTINEL_TIMEEVENT_H
#define FLYSENTINEL_TIMEEVENT_H

#include "../coordinator/CoordinatorDef.h"

int64_t getCurrentTime();

struct TimeEvent {
public:
    TimeEvent(int64_t id,
              int64_t milliseconds,
              timeEventProc *proc,
              void *clientData,
              eventFinalizerProc *finalizerProc);
    int64_t getId() const;
    void setId(int64_t id);
    int64_t getWhen() const;
    void setWhen(int64_t milliseconds);
    void *getClientData();
    eventFinalizerProc* getFinalizerProc();
    timeEventProc* getTimeProc();
    bool operator< (const TimeEvent& timeEvent) const;
    bool operator== (const TimeEvent& timeEvent) const;

    int64_t id;
    int64_t when;         // 单位：ms
    timeEventProc *timeProc;
    eventFinalizerProc *finalizerProc;
    void *clientData;
};

#endif //FLYSENTINEL_TIMEEVENT_H
