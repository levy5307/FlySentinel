//
// Created by 赵立伟 on 2018/12/3.
//

#include "FlyClientFactory.h"
#include "FlyClient.h"

FlyClientFactory::FlyClientFactory(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

std::shared_ptr<AbstractFlyClient> FlyClientFactory::getFlyClient(int fd,
                                                                  time_t nowt) {
    return std::shared_ptr<AbstractFlyClient>(new FlyClient(fd, this->coordinator, nowt));
}
