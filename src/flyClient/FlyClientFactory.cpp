//
// Created by 赵立伟 on 2018/12/3.
//

#include "FlyClientFactory.h"
#include "FlyClient.h"

std::shared_ptr<AbstractFlyClient> FlyClientFactory::getFlyClient(int fd,
                                                                  const AbstractCoordinator *coordinator,
                                                                  AbstractFlyDB *flyDB,
                                                                  time_t nowt) {
    return std::shared_ptr<AbstractFlyClient>(new FlyClient(fd, coordinator, flyDB, nowt));
}
