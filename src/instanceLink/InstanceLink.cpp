//
// Created by levy on 2019/5/5.
//

#include "InstanceLink.h"
#include "../def.h"

InstanceLink::InstanceLink() {
    uint64_t nowt = miscTool->mstime();
    this->lastAvailTime = nowt;
    this->actPingTime = nowt;
    this->lastPongTime = nowt;
}

InstanceLink::~InstanceLink() {
    this->commandContext = NULL;
    this->pubsubContext = NULL;
}

void InstanceLink::closeConnection(std::shared_ptr<redisAsyncContext> context) {
    if (this->commandContext == context) {
        this->commandContext = NULL;
        this->pendingCommands = 0;
    }

    if (this->pubsubContext == context) {
        this->pubsubContext = NULL;
    }

    context->data = NULL;
    this->disconnected = 1;
    flyAsyncFree(context.get());
}
