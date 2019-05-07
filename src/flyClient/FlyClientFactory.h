//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_FLYCLIENTFACTORY_H
#define FLYSENTINEL_FLYCLIENTFACTORY_H

#include "../coordinator/interface/AbstractFlyClientFactory.h"

class FlyClientFactory : public AbstractFlyClientFactory {
public:
    std::shared_ptr<AbstractFlyClient> getFlyClient(
            int fd, 
            const AbstractCoordinator *coordinator,
            time_t nowt);
};


#endif //FLYSENTINEL_FLYCLIENTFACTORY_H