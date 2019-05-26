//
// Created by levy on 2019/5/26.
//

#include "SentinelAddr.h"

SentinelAddr::SentinelAddr(const std::string &ip, int port) {
    this->ip = ip;
    this->port = port;
}

SentinelAddr* SentinelAddr::operator=(const SentinelAddr *sa) {
    return new SentinelAddr(sa->ip, sa->port);
}

bool SentinelAddr::operator==(const SentinelAddr *sa) {
    return sa->port == this->port && sa->ip == this->ip;
}

const std::string& SentinelAddr::getIp() const {
    return ip;
}

int SentinelAddr::getPort() const {
    return port;
}

