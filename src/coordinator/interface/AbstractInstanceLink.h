//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_ABSTRACTINSTANCELINK_H
#define FLYSENTINEL_ABSTRACTINSTANCELINK_H

#include "../../asyncEvent/convert.h"

class AbstractInstanceLink {
public:
    virtual ~AbstractInstanceLink() {};
    virtual void closeConnection(std::shared_ptr<redisAsyncContext> context) = 0;
    virtual const std::shared_ptr<redisAsyncContext> &getCommandContext() const = 0;
    virtual const std::shared_ptr<redisAsyncContext> &getPubsubContext() const = 0;
};

#endif //FLYSENTINEL_ABSTRACTINSTANCELINK_H
