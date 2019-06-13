//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_ABSTRACTFLYCLIENTFACTORY_H
#define FLYSENTINEL_ABSTRACTFLYCLIENTFACTORY_H

#include <memory>
#include "AbstractFlyClient.h"

class AbstractFlyClientFactory {
public:
    virtual std::shared_ptr<AbstractFlyClient> getFlyClient(
            int fd,
            time_t nowt) = 0;
};

#endif //FLYSENTINEL_ABSTRACTFLYCLIENTFACTORY_H
