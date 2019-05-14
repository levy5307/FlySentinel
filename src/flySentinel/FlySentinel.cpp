//
// Created by levy on 2019/5/5.
//

#include <iostream>
#include "FlySentinel.h"
#include "../flyClient/ClientDef.h"
#include "../flyClient/FlyClient.h"
#include "../def.h"
#include "../dataStructure/dict/Dict.cpp"

FlySentinel::FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache)
        : AbstractFlyServer(coordinator, configCache) {
    memset(this->myid, 0, sizeof(this->myid));
    this->masters = new Dict<std::string, std::shared_ptr<AbstractFlyInstance>>();
}

FlySentinel::~FlySentinel() {

}

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    /** 更新缓存时间 */
    flyServer->setNowt(time(NULL));

    /** 释放所有异步删除的clients */
    flyServer->freeClientsInAsyncFreeList();

    /** cron loop static */
    flyServer->addCronLoops();
    std::cout << "serverCron is running "
              << flyServer->getCronLoops()
              << " times!" << std::endl;
    return 1000 / flyServer->getHz();
}
