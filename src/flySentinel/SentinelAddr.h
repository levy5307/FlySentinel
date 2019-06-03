//
// Created by levy on 2019/5/26.
//

#ifndef FLYSENTINEL_SENTINELADDR_H
#define FLYSENTINEL_SENTINELADDR_H

#include <string>

class SentinelAddr {
public:
    SentinelAddr(const std::string &ip, int port);
    SentinelAddr* operator=(const SentinelAddr &sa);
    bool operator==(const SentinelAddr &sa);
    bool operator!=(const SentinelAddr &sa);
    const std::string& getIp() const;
    int getPort() const;
    void setIp(const std::string &ip);
    void setPort(int port);

    std::string ip;
    int port = 0;
};


#endif //FLYSENTINEL_SENTINELADDR_H
