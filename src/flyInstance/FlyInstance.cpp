//
// Created by levy on 2019/5/5.
//

#include "FlyInstance.h"
#include "../flySentinel/FlySentinelDef.h"
#include "../def.h"
#include "FlyInstanceDef.h"

FlyInstance::FlyInstance() {

}

FlyInstance::~FlyInstance() {
    if (NULL != this->addr) {
        delete this->addr;
    }

    this->master = NULL;
    this->sentinels.clear();
    this->sentinels.clear();
    this->link = NULL;
}

int FlyInstance::getFlags() const {
    return flags;
}

void FlyInstance::setFlags(int flags) {
    this->flags = flags;
}

const std::string &FlyInstance::getName() const {
    return name;
}

void FlyInstance::setName(const std::string &name) {
    this->name = name;
}

SentinelAddr *FlyInstance::getAddr() const {
    return addr;
}

void FlyInstance::setAddr(SentinelAddr *addr) {
    this->addr = addr;
}

void FlyInstance::dupAddr(SentinelAddr *addr) {
    if (NULL == this->addr) {
        this->addr = new SentinelAddr(addr->getIp(), addr->getPort());
        return;
    }

    this->addr->setIp(addr->getIp());
    this->addr->setPort(addr->getPort());
}

std::shared_ptr<AbstractFlyInstance> FlyInstance::getMaster() const {
    return this->master;
}

bool FlyInstance::haveMaster() const {
    return 0 != this->flags & FSI_MASTER;
}

void FlyInstance::setMaster(std::shared_ptr<AbstractFlyInstance> master) {
    this->master = master;
}

uint32_t FlyInstance::getQuorum() const {
    return quorum;
}

void FlyInstance::setQuorum(uint32_t quorum) {
    this->quorum = quorum;
}

char *FlyInstance::getNotificationScript() const {
    return notificationScript;
}

void FlyInstance::setNotificationScript(char *notificationScript) {
    this->notificationScript = notificationScript;
}

char *FlyInstance::getClientReconfigScript() const {
    return clientReconfigScript;
}

bool FlyInstance::isClientReconfigScriptNULL() const {
    return NULL == this->clientReconfigScript;
}

const std::shared_ptr<AbstractInstanceLink> &FlyInstance::getLink() const {
    return link;
}

void FlyInstance::setLink(const std::shared_ptr<AbstractInstanceLink> &link) {
    this->link = link;
}

void FlyInstance::releaseLink() {
    if (link.use_count() > 1) {
        if (this->link->getCommandContext()) {
            redisCallbackList *callbacks = &link->getCommandContext()->replies;
            redisCallback *callback = callbacks->head;
            while (NULL != callback) {
                if (this == callback->privdata) {
                    callback->fn = sentinelDiscardReplyCallback;
                    callback->privdata = NULL;
                }
                callback = callback->next;
            }
        }
    }

    this->link = NULL;
}

const std::string &FlyInstance::getRunid() const {
    return runid;
}

void FlyInstance::setRunid(const std::string &runid) {
    this->runid = runid;
}

const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &FlyInstance::getSentinels() const {
    return sentinels;
}

const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &FlyInstance::getSlaves() const {
    return slaves;
}

std::shared_ptr<AbstractFlyInstance> FlyInstance::lookupSlave(char *ip, int port) {
    std::string addr = miscTool->formatAddr(ip, port);
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>>::const_iterator citer = this->slaves.find(addr);
    if (citer != this->slaves.end()) {
        return citer->second;
    }

    return NULL;
}

int FlyInstance::removeMatchingSentinel(char *runid) {
    /** 移除数量 */
    int removed = 0;

    /** 遍历查找并删除 */
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>>::iterator iter = this->sentinels.begin();
    for (iter; iter != this->sentinels.end(); iter++) {
        if (0 == iter->second->getRunid().compare(runid)) {
            iter = this->sentinels.erase(iter);
            removed++;
        }
    }

    return removed;
}

void FlyInstance::reset(int flags) {
    this->slaves.clear();
    if (flags & SENTINEL_RESET_SENTINELS) {
        this->sentinels.clear();
    }
    this->flags &= FSI_MASTER | FSI_SLAVE | FSI_SENTINEL;
    this->runid.clear();
    this->master = NULL;
    this->link.reset();
}

void sentinelDiscardReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractInstanceLink *instanceLink = (AbstractInstanceLink *)context->data;
    if (NULL != instanceLink) {
        instanceLink->decreasePendingCommands();
    }
}
