//
// Created by 赵立伟 on 2018/10/20.
//

#include "TimeEvent.h"

TimeEvent::TimeEvent(int64_t id,
                     int64_t milliseconds,
                     timeEventProc *proc,
                     void *clientData,
                     eventFinalizerProc *finalizerProc) {
    this->id = id;
    this->timeProc = proc;
    this->clientData = clientData;
    this->finalizerProc = finalizerProc;
    this->when = milliseconds;
}

int64_t TimeEvent::getId() const {
    return this->id;
}

void TimeEvent::setId(int64_t id) {
    this->id = id;
}

int64_t TimeEvent::getWhen() const {
    return this->when;
}

void TimeEvent::setWhen(int64_t milliseconds) {
    this->when = milliseconds;
}

void *TimeEvent::getClientData() {
    return this->clientData;
}

eventFinalizerProc* TimeEvent::getFinalizerProc() {
    return this->finalizerProc;
}

timeEventProc* TimeEvent::getTimeProc() {
    return this->timeProc;
}

bool TimeEvent::operator< (const TimeEvent& timeEvent) const {
    return this->when < timeEvent.when;
}

bool TimeEvent::operator== (const TimeEvent& timeEvent) const {
    return this->when == timeEvent.when;
}

int64_t getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

