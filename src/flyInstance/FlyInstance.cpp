//
// Created by levy on 2019/5/5.
//

#include "FlyInstance.h"
#include "../flySentinel/FlySentinelDef.h"
#include "../def.h"
#include "FlyInstanceDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../net/NetDef.h"

extern AbstractCoordinator *coordinator;

FlyInstance::FlyInstance(const std::string &name, int flags, const std::string &hostname,
                         int port, int quorum, std::shared_ptr<AbstractFlyInstance> master) {
    uint64_t nowt = miscTool->mstime();
    this->name = name;
    this->flags = flags;
    this->addr = new SentinelAddr(hostname, port);
    this->quorum = quorum;
    this->master = master;
    this->downAfterPeriod = this->master ? this->master->getDownAfterPeriod() : SENTINEL_DEFAULT_DOWN_AFTER;
    this->roleReported = this->flags & (FSI_MASTER | FSI_SLAVE);
    this->roleReportedTime = nowt;
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
    if (NULL != this->addr) {
        delete this->addr;
    }
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
    uint64_t nowt = miscTool->mstime();
    this->slaves.clear();
    if (flags & SENTINEL_RESET_SENTINELS) {
        this->sentinels.clear();
    }
    this->flags &= FSI_MASTER | FSI_SLAVE | FSI_SENTINEL;
    this->runid.clear();
    this->master = NULL;
    this->link.reset();
    this->oDownSinceTime = 0;
    this->sDownSinceTime = 0;
    this->promotedSlave = NULL;
    this->failoverState = SENTINEL_FAILOVER_STATE_NONE;
    this->roleReported = this->flags & (FSI_MASTER | FSI_SLAVE);
    this->roleReportedTime = nowt;
}

bool FlyInstance::noDownFor(uint64_t ms) {
    /** 取sdown和odown两者中最大 */
    uint64_t mostRecent = this->oDownSinceTime > this->sDownSinceTime ? this->oDownSinceTime : this->sDownSinceTime;
    return 0 == mostRecent || miscTool->mstime() - ms > mostRecent;
}

uint64_t FlyInstance::getDownAfterPeriod() const {
    return downAfterPeriod;
}

void FlyInstance::setDownAfterPeriod(uint64_t downAfterPeriod) {
    this->downAfterPeriod = downAfterPeriod;
}

int FlyInstance::getRoleReported() const {
    return roleReported;
}

void FlyInstance::setRoleReported(int roleReported) {
    this->roleReported = roleReported;
}

uint64_t FlyInstance::getRoleReportedTime() const {
    return roleReportedTime;
}

void FlyInstance::setRoleReportedTime(uint64_t roleReportedTime) {
    this->roleReportedTime = roleReportedTime;
}

uint64_t FlyInstance::getInfoRefresh() const {
    return infoRefresh;
}

void FlyInstance::setInfoRefresh(uint64_t infoRefresh) {
    this->infoRefresh = infoRefresh;
}

bool FlyInstance::sendPing() {
    int retval = redisAsyncCommand(this->link->getCommandContext().get(), NULL, this, "%s", "PING");
    if (retval > 0) {
        this->link->increasePendingCommands();
        this->link->setLastPingTime(miscTool->mstime());
        if (0 == this->link->getActPingTime()) {
            this->link->setActPingTime(this->link->getLastPingTime());
        }
        return true;
    } else {
        return false;
    }
}

int FlyInstance::sendHello() {
    /** 这里的master不能用智能指针，因为this再放入一个智能指针里，有可能会被释放两次 */
    AbstractFlyInstance *master = (this->flags & FSI_MASTER) ? this : this->getMaster().get();
    SentinelAddr *masterAddr = master->getAddr();
    AbstractFlyServer *flyServer = coordinator->getFlyServer();

    /** 如果连接已经断开了，直接返回 */
    if (this->getLink()->isDisconnected()) {
        return -1;
    }

    /** 获取announce ip和port, 如果没有设置用于gossip协议的addr，则使用命令连接的addr */
    std::string announceIP = flyServer->getAnnounceIP();
    if (announceIP.empty()) {
        int fd = this->getLink()->getCommandContext()->c.fd;
        if (-1 == coordinator->getNetHandler()->sockName(fd, (char*)announceIP.c_str(), NET_IP_STR_LEN, NULL)) {
            return -1;
        }
    }
    /** 如果用于gossip的port无效，则使用监听port */
    int announcePort = (0 == flyServer->getAnnouncePort()) ?  flyServer->getPort() : flyServer->getAnnouncePort();

    /** 按照格式获取hello message的数据 */
    char payload[NET_IP_STR_LEN+1024];
    snprintf(payload, sizeof(payload),
             "%s,%d,%s,%llu," /** Info about this sentinel. */
             "%s,%s,%d,%llu", /** Info about current master. */
             announceIP.c_str(), announcePort, flyServer->getMyid(), (unsigned long long) flyServer->getCurrentEpoch(),
             master->getName().c_str(), masterAddr->ip.c_str(), masterAddr->port, (unsigned long long) master->getConfigEpoch());

    /** 发送该hello message */
    if(-1 == redisAsyncCommand(this->link->getCommandContext().get(), sentinelPublishReplyCallback,
                               this, "%s %s %s", "PUBLISH", SENTINEL_HELLO_CHANNEL.c_str(), payload)) {
        return -1;
    }

    /** 增加pending command数量 */
    this->link->increasePendingCommands();
    return 1;
}

void FlyInstance::sendPeriodicCommands() {
    
}

const std::shared_ptr<AbstractFlyInstance> &FlyInstance::getPromotedSlave() const {
    return promotedSlave;
}

void FlyInstance::setPromotedSlave(const std::shared_ptr<AbstractFlyInstance> &promotedSlave) {
    this->promotedSlave = promotedSlave;
}

bool FlyInstance::hasPromotedSlave() const {
    return NULL != this->promotedSlave;
}

FailoverState FlyInstance::getFailoverState() const {
    return failoverState;
}

void FlyInstance::setFailoverState(FailoverState failoverState) {
    this->failoverState = failoverState;
}

void FlyInstance::clearInfo() {
    this->info.clear();
}

void FlyInstance::setInfo(const std::string &info) {
    this->info = info;
}

const std::string& FlyInstance::getInfo() const {
    return this->info;
}

uint64_t FlyInstance::getConfigEpoch() const {
    return configEpoch;
}

void FlyInstance::setConfigEpoch(uint64_t configEpoch) {
    this->configEpoch = configEpoch;
}

uint64_t FlyInstance::getLastPubTime() const {
    return this->lastPubTime;
}

void FlyInstance::setLastPubTime(uint64_t lastPubTime) {
    this->lastPubTime = lastPubTime;
}

uint64_t FlyInstance::getLastHelloTime() const {
    return lastHelloTime;
}

void FlyInstance::setLastHelloTime(uint64_t lastHelloTime) {
    this->lastHelloTime = lastHelloTime;
}

void sentinelDiscardReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractInstanceLink *instanceLink = (AbstractInstanceLink *)context->data;
    if (NULL != instanceLink) {
        instanceLink->decreasePendingCommands();
    }
}

void sentinelInfoReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractFlyInstance *flyInstance = (AbstractFlyInstance *)privdata;
    AbstractInstanceLink *instanceLink = (AbstractInstanceLink *)context->data;
    if (NULL == reply || NULL == instanceLink) {
        return;
    }

    instanceLink->increasePendingCommands();
    redisReply *r = (redisReply*)reply;
    if (REDIS_REPLY_STRING == r->type) {
        coordinator->getFlyServer()->refreshInstanceInfo(flyInstance, r->str);
    }
}

void sentinelPublishReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractFlyInstance *flyInstance = (AbstractFlyInstance *)privdata;
    AbstractInstanceLink *instanceLink = (AbstractInstanceLink *)context->data;
    if (NULL == reply || NULL == instanceLink) {
        return;
    }

    /** 减少instance link的pending command数量 */
    instanceLink->increasePendingCommands();

    /**
     * 如果发送失败了，则不更新最后一次发送时间，以便于下一次loop时重新发送;
     * 否则，发送成功了则需要更新最后一次的发送时间
     **/
    redisReply *r = (redisReply*)reply;
    if (REDIS_REPLY_ERROR != r->type) {
        flyInstance->setLastPubTime(miscTool->mstime());
    }
}
