//
// Created by 赵立伟 on 2018/10/20.
//

#include <cstdio>
#include "FileEvent.h"
#include "Select.h"
#include "EventDef.h"

FileEvent::FileEvent() {
}

void FileEvent::setMask(int mask) {
    this->mask = mask;
}

int FileEvent::getMask() const {
    return this->mask;
}

int FileEvent::addFileProc(int mask,
                           fileEventProc *proc,
                           void *privdata) {
    // 设置相应的proc
    this->mask |= mask;
    if (mask & ES_READABLE) {
        this->rfileProc = proc;
    }
    if (mask & ES_WRITABLE) {
        this->wfileProc = proc;
    }
    this->privdata = privdata;

    return 1;
}

void FileEvent::delFileProc(int mask) {
    this->mask &= ~mask;
}

bool FileEvent::noneMask() {
    return ES_NONE == this->mask;
}

void FileEvent::process(int mask) {
    int rfired = 0;

    // 如果是有可读/可写事件，则执行事件回调
    if (mask & ES_READABLE) {
        rfired = 1;
        rfileProc(this->coordinator, this->fd, this->privdata, mask);
    }
    if (mask & ES_WRITABLE) {
        if (this->wfileProc != this->rfileProc || 0 == rfired) {
            wfileProc(this->coordinator, fd, this->privdata, mask);
        }
    }
}

int FileEvent::getFd() const {
    return this->fd;
}

void FileEvent::setFd(int fd) {
    this->fd = fd;
}

void FileEvent::setCoordinator(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

