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
    closeConnection(this->commandContext);
    closeConnection(this->pubsubContext);
}

void InstanceLink::reset() {
    uint64_t nowt = miscTool->mstime();
    this->disconnected = true;
    this->pendingCommands = 0;
    this->ccConnTime = 0;
    this->pcConntime = 0;
    this->pcLastActivity = 0;
    this->lastAvailTime = nowt;
    this->actPingTime = nowt;
    this->lastPingTime = 0;
    this->lastPongTime = nowt;
    this->lastReconnTime = 0;
    closeConnection(this->commandContext);
    closeConnection(this->pubsubContext);
}

void InstanceLink::closeConnection(const std::shared_ptr<redisAsyncContext> context) {
    if (NULL == context) {
        return;
    }

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

const std::shared_ptr<redisAsyncContext> &InstanceLink::getCommandContext() const {
    return commandContext;
}

const std::shared_ptr<redisAsyncContext> &InstanceLink::getPubsubContext() const {
    return pubsubContext;
}

void InstanceLink::decreasePendingCommands() {
    this->pendingCommands--;
}

void InstanceLink::increasePendingCommands() {
    this->pendingCommands++;
}

void InstanceLink::connectionError(const std::shared_ptr<redisAsyncContext> context) {
    if (NULL == context) {
        return;
    }

    if (this->commandContext == context) {
        this->commandContext = NULL;
    } else {
        this->pubsubContext = NULL;
    }
    disconnected = 1;
}

uint64_t InstanceLink::getActPingTime() const {
    return actPingTime;
}

void InstanceLink::setActPingTime(uint64_t actPingTime) {
    this->actPingTime = actPingTime;
}

uint64_t InstanceLink::getLastPingTime() const {
    return lastPingTime;
}

void InstanceLink::setLastPingTime(uint64_t lastPingTime) {
    this->lastPingTime = lastPingTime;
}

bool InstanceLink::isDisconnected() const {
    return disconnected;
}

void InstanceLink::setDisconnected(bool disconnected) {
    this->disconnected = disconnected;
}

int InstanceLink::getPendingCommands() const {
    return pendingCommands;
}

uint64_t InstanceLink::getLastPongTime() const {
    return lastPongTime;
}

void InstanceLink::setLastPongTime(uint64_t lastPongTime) {
    this->lastPongTime = lastPongTime;
}

uint64_t InstanceLink::getPcLastActivity() const {
    return pcLastActivity;
}

void InstanceLink::setPcLastActivity(uint64_t pcLastActivity) {
    this->pcLastActivity = pcLastActivity;
}

uint64_t InstanceLink::getLastAvailTime() const {
    return lastAvailTime;
}

void InstanceLink::setLastAvailTime(uint64_t lastAvailTime) {
    this->lastAvailTime = lastAvailTime;
}
