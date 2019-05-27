//
// Created by levy on 2019/5/5.
//

#include "FlyDBInstance.h"
#include "../flySentinel/FlysentinelDef.h"

FlyDBInstance::FlyDBInstance() {

}

FlyDBInstance::~FlyDBInstance() {
    if (NULL != this->addr) {
        delete this->addr;
    }

    this->master = NULL;
}

int FlyDBInstance::getFlags() const {
    return flags;
}

void FlyDBInstance::setFlags(int flags) {
    this->flags = flags;
}

const std::string &FlyDBInstance::getName() const {
    return name;
}

void FlyDBInstance::setName(const std::string &name) {
    this->name = name;
}

SentinelAddr *FlyDBInstance::getAddr() const {
    return addr;
}

void FlyDBInstance::setAddr(SentinelAddr *addr) {
    this->addr = addr;
}

std::shared_ptr<AbstractFlyDBInstance> FlyDBInstance::getMaster() const {
    return this->master;
}

bool FlyDBInstance::haveMaster() const {
    return 0 != this->flags & FSI_MASTER;
}

void FlyDBInstance::setMaster(std::shared_ptr<AbstractFlyDBInstance> master) {
    this->master = master;
}

uint32_t FlyDBInstance::getQuorum() const {
    return quorum;
}

void FlyDBInstance::setQuorum(uint32_t quorum) {
    this->quorum = quorum;
}

char *FlyDBInstance::getNotificationScript() const {
    return notificationScript;
}

void FlyDBInstance::setNotificationScript(char *notificationScript) {
    this->notificationScript = notificationScript;
}
