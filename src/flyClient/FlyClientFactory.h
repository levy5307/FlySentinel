//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_FLYCLIENTFACTORY_H
#define FLYDB_FLYCLIENTFACTORY_H

#include "../coordinator/interface/AbstractFlyClientFactory.h"

class FlyClientFactory : public AbstractFlyClientFactory {
public:
    std::shared_ptr<AbstractFlyClient> getFlyClient(
            int fd, 
            const AbstractCoordinator *coordinator,
            AbstractFlyDB *flyDB,
            time_t nowt);
};


#endif //FLYDB_FLYCLIENTFACTORY_H
