//
// Created by levy on 2019/5/5.
//

#include <iostream>
#include "FlySentinel.h"
#include "../flyClient/ClientDef.h"
#include "../flyClient/FlyClient.h"
#include "../def.h"

FlySentinel::FlySentinel(const AbstractCoordinator *coordinator) : AbstractFlyServer(coordinator) {
}

FlySentinel::~FlySentinel() {

}

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData) {

}
