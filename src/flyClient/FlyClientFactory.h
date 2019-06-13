//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_FLYCLIENTFACTORY_H
#define FLYSENTINEL_FLYCLIENTFACTORY_H

#include "../coordinator/interface/AbstractFlyClientFactory.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class FlyClientFactory : public AbstractFlyClientFactory {
public:
    FlyClientFactory(const AbstractCoordinator *coordinator);
    std::shared_ptr<AbstractFlyClient> getFlyClient(
            int fd, 
            time_t nowt);

private:
    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_FLYCLIENTFACTORY_H
