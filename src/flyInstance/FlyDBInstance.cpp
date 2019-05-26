//
// Created by levy on 2019/5/5.
//

#include "FlyDBInstance.h"

FlyDBInstance::FlyDBInstance() {

}

FlyDBInstance::~FlyDBInstance() {
    if (NULL != this->addr) {
        delete this->addr;
    }
    
    if (NULL != this->master) {
        delete this->master;
    }
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

AbstractFlyDBInstance *FlyDBInstance::getMaster() const {
    return master;
}

void FlyDBInstance::setMaster(AbstractFlyDBInstance *master) {
    this->master = master;
}
