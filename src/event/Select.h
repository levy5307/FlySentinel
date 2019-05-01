//
// Created by 赵立伟 on 2018/10/20.
//

#ifndef FLYDB_SELECT_H
#define FLYDB_SELECT_H

#include <sys/select.h>
#include <map>

class PollState {
public:
    PollState(const AbstractCoordinator *coordinator);
    void add(int fd, int mask);
    void del(int fd, int mask);
    void poll(struct timeval *tvp, std::map<int, int> &res);

private:
    fd_set rfds, wfds;
    // 当select()之后再去使用fd会不安全, 因此保留一个fd的备份
    fd_set _rfds, _wfds;

    AbstractEventLoop *eventLoop;
};

#endif //FLYDB_SELECT_H
