//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_ABSTRACTINSTANCELINK_H
#define FLYSENTINEL_ABSTRACTINSTANCELINK_H

#include <memory>
#include "../../asyncEvent/convert.h"

class AbstractInstanceLink {
public:
    virtual ~AbstractInstanceLink() {};
    virtual void reset() = 0;
    virtual void closeConnection(const std::shared_ptr<redisAsyncContext> context) = 0;
    virtual const std::shared_ptr<redisAsyncContext> &getCommandContext() const = 0;
    virtual const std::shared_ptr<redisAsyncContext> &getPubsubContext() const = 0;
    virtual void decreasePendingCommands() = 0;
    virtual void increasePendingCommands() = 0;
    virtual void connectionError(const std::shared_ptr<redisAsyncContext> context) = 0;
    virtual uint64_t getActPingTime() const = 0;
    virtual void setActPingTime(uint64_t actPingTime) = 0;
    virtual uint64_t getLastPingTime() const = 0;
    virtual void setLastPingTime(uint64_t lastPingTime) = 0;
};

#endif //FLYSENTINEL_ABSTRACTINSTANCELINK_H
