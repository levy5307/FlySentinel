//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_FLYSENTINELDEF_H
#define FLYSENTINEL_FLYSENTINELDEF_H

const int FLYDB_SENTINEL_PORT = 26379;

class SentinelAddr {
    SentinelAddr(const std::string &ip, int port) {
        this->ip = ip;
        this->port = port;
    }

    SentinelAddr* operator=(const SentinelAddr *sa) {
        return new SentinelAddr(sa->ip, sa->port);
    }

    bool operator==(const SentinelAddr *sa) {
        return sa->port == this->port && sa->ip == this->ip;
    }

    std::string ip;
    int port = 0;
};


#endif //FLYSENTINEL_FLYSENTINELDEF_H
