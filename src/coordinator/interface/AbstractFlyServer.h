//
// Created by levy on 2019/5/7.
//

#ifndef FLYSENTINEL_ABSTRACTFLYSERVER_H
#define FLYSENTINEL_ABSTRACTFLYSERVER_H

#include "AbstractFlyClient.h"

class ConfigCache;

class AbstractFlyServer {
public:
    virtual ~AbstractFlyServer() {};
    virtual void init(ConfigCache *configCache) = 0;
    virtual int handleClientsWithPendingWrites() = 0;
    virtual void freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual void freeClientAsync(int fd) = 0;
    virtual int getMaxClients() const = 0;
};

#endif //FLYSENTINEL_ABSTRACTFLYSERVER_H
