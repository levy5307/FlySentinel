//
// Created by levy on 2019/5/5.
//

#include <cassert>
#include "FlyInstance.h"
#include "FlyInstanceDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"

extern AbstractCoordinator *coordinator;

FlyInstance::FlyInstance(const std::string &name, int flags, const std::string &hostname,
                         int port, uint32_t quorum, AbstractFlyInstance* master) {
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

void FlyInstance::addFlags(int flags) {
    this->flags |= flags;
}

void FlyInstance::delFlags(int flags) {
    this->flags &= ~flags;
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

void FlyInstance::setPort(int port) {
    this->addr->setPort(port);
}

const std::string &FlyInstance::getIP() const {
    return this->addr->getIp();
}

int FlyInstance::getPort() const {
    return this->addr->getPort();
}

AbstractFlyInstance* FlyInstance::getMaster() const {
    return this->master;
}

bool FlyInstance::haveMaster() const {
    return 0 != this->flags & FSI_MASTER;
}

void FlyInstance::setMaster(AbstractFlyInstance* master) {
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

const std::map<std::string, AbstractFlyInstance*> &FlyInstance::getSentinels() const {
    return sentinels;
}

const std::map<std::string, AbstractFlyInstance*> &FlyInstance::getSlaves() const {
    return slaves;
}

AbstractFlyInstance* FlyInstance::lookupSlave(char *ip, int port) {
    std::string addr = miscTool->formatAddr(ip, port);
    std::map<std::string, AbstractFlyInstance*>::const_iterator citer = this->slaves.find(addr);
    if (citer != this->slaves.end()) {
        return citer->second;
    }

    return NULL;
}

int FlyInstance::removeMatchingSentinel(const std::string &runid) {
    /** 移除数量 */
    int removed = 0;

    /** 遍历查找并删除 */
    std::map<std::string, AbstractFlyInstance*>::iterator iter = this->sentinels.begin();
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

    /** clear slaves */
    std::map<std::string, AbstractFlyInstance*>::iterator iter = this->slaves.begin();
    for (iter; iter != this->slaves.end(); iter++) {
        delete iter->second;
    }
    this->slaves.clear();

    /** clear sentinels */
    if (flags & SENTINEL_RESET_SENTINELS) {
        std::map<std::string, AbstractFlyInstance*>::iterator iter = this->sentinels.begin();
        for (iter; iter != this->sentinels.end(); iter++) {
            delete iter->second;
        }
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


AbstractFlyInstance* FlyInstance::getPromotedSlave() const {
    return promotedSlave;
}

void FlyInstance::setPromotedSlave(AbstractFlyInstance* promotedSlave) {
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
    this->failoverStateChangeTime = miscTool->mstime();
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

void FlyInstance::addReplySentinelRedisInstance(std::shared_ptr<AbstractFlyClient> flyClient) {
    /** name pair */
    flyClient->addReplyBulkString("name");
    flyClient->addReplyBulkString(this->name);

    /** ip pair */
    flyClient->addReplyBulkString("ip");
    flyClient->addReplyBulkString(this->addr->getIp());

    /** port pair */
    flyClient->addReplyBulkString("port");
    flyClient->addReplyLongLong(this->addr->getPort());

    /** runid pair */
    flyClient->addReplyBulkString("runid");
    flyClient->addReplyBulkString(this->runid);

    /** flags */
    flyClient->addReplyBulkString("flags");
    flyClient->addReplyBulkString(this->getFlagsString());

    /** pending commands */
    flyClient->addReplyBulkString("link-pending-commands");
    flyClient->addReplyBulkLongLong(this->getLink()->getPendingCommands());

    /** link reference count */
    flyClient->addReplyBulkString("link-refcount");
    flyClient->addReplyBulkLongLong(this->getLink().use_count());
}

int FlyInstance::sentinelIsQuorumReachable(int *usablePtr) {
    assert(this->flags & FSI_MASTER);

    int usable = 1;
    int totalVoters = this->sentinels.size() + 1;   /** 总票数 = sentinel数量 + 1 */
    for (auto iter : this->sentinels) {
        if (iter.second->getFlags() & (FSI_O_DOWN | FSI_S_DOWN)) {
            continue;
        }

        usable++;
    }

    int res = FSQS_OK;
    /** 如果可用票数 < 配置最少票数，返回 */
    if (usable < this->quorum) {
        res |= FSQS_NOQUORUM;
    }

    /** 如果可用票数 < 总票数 / 2 + 1，返回 */
    if (usable < totalVoters / 2 + 1) {
        res |= FSQS_NOVOTER;
    }

    if (NULL != usablePtr) {
        *usablePtr = usable;
    }

    return res;
}

std::string FlyInstance::getFlagsString() {
    std::string flags;
    if (this->flags & FSI_S_DOWN) {
        flags += "s_down,";
    }
    if (this->flags & FSI_O_DOWN) {
        flags += "o_down,";
    }
    if (this->flags & FSI_MASTER) {
        flags += "master,";
    }
    if (this->flags & FSI_SLAVE) {
        flags += "slave,";
    }
    if (this->flags & FSI_SENTINEL) {
        flags += "sentinel,";
    }
    if (this->link->isDisconnected()) {
        flags += "disconnected,";
    }
    if (this->flags & FSI_MASTER_DOWN) {
        flags += "master_down,";
    }
    if (this->flags & FSI_FAILOVER_IN_PROGRESS) {
        flags += "failover_in_progress,";
    }
    if (this->flags & FSI_PROMOTED) {
        flags += "promoted,";
    }
    if (this->flags & FSI_RECONF_SENT) {
        flags += "reconf_sent,";
    }
    if (this->flags & FSI_RECONF_INPROG) {
        flags += "reconf_inprog,";
    }
    if (this->flags & FSI_RECONF_DONE) {
        flags += "reconf_done,";
    }

    /** 如果非空，则需要去除掉最后的","*/
    if (!flags.empty()) {
        flags = flags.substr(0, flags.size() - 1);
    }

    return flags;
}

uint64_t FlyInstance::getMasterLinkDownTime() const {
    return masterLinkDownTime;
}

void FlyInstance::setMasterLinkDownTime(uint64_t masterLinkDownTime) {
    this->masterLinkDownTime = masterLinkDownTime;
}

const std::string &FlyInstance::getSlaveMasterHost() const {
    return this->slaveMasterHost;
}

void FlyInstance::setSlaveMasterHost(const std::string &slaveMasterHost) {
    this->slaveMasterHost = slaveMasterHost;
    this->slaveConfChangeTime = miscTool->mstime();
}

int FlyInstance::getSlaveMasterPort() const {
    return this->slaveMasterPort;
}

void FlyInstance::setSlaveMasterPort(int slaveMasterPort) {
    this->slaveMasterPort = slaveMasterPort;
    this->slaveConfChangeTime = miscTool->mstime();
}

int FlyInstance::getSlaveMasterLinkStatus() const {
    return slaveMasterLinkStatus;
}

void FlyInstance::setSlaveMasterLinkStatus(int slaveMasterLinkStatus) {
    this->slaveMasterLinkStatus = slaveMasterLinkStatus;
}

int FlyInstance::getSlavePriority() const {
    return slavePriority;
}

void FlyInstance::setSlavePriority(int slavePriority) {
    this->slavePriority = slavePriority;
}

uint64_t FlyInstance::getSlaveReplOffset() const {
    return slaveReplOffset;
}

void FlyInstance::setSlaveReplOffset(uint64_t slaveReplOffset) {
    this->slaveReplOffset = slaveReplOffset;
}

uint64_t FlyInstance::getSlaveConfChangeTime() const {
    return slaveConfChangeTime;
}

void FlyInstance::setSlaveConfChangeTime(uint64_t slaveConfChangeTime) {
    this->slaveConfChangeTime = slaveConfChangeTime;
}

bool FlyInstance::isPromotedSlave() {
   return (this->getFlags() & FSI_PROMOTED)
          && (this->getMaster()->getFlags() & FSI_FAILOVER_IN_PROGRESS)
          && (this->getMaster()->getFailoverState() == SENTINEL_FAILOVER_STATE_WAIT_PROMOTION);
}

uint64_t FlyInstance::getFailoverEpoch() const {
    return this->failoverEpoch;
}

void FlyInstance::setFailoverEpoch(uint64_t failoverEpoch) {
    this->failoverEpoch = failoverEpoch;
}

int FlyInstance::forceHelloUpdate() {
    if (0 == this->flags & FSI_MASTER) {
        return -1;
    }

    if (this->lastPubTime >= SENTINEL_PUBLISH_PERIOD + 1) {
        this->lastPubTime -= (SENTINEL_PUBLISH_PERIOD + 1);
    }

    this->forceHelloUpdateFlyInstances(this->sentinels);
    this->forceHelloUpdateFlyInstances(this->slaves);

    return 1;
}

void FlyInstance::forceHelloUpdateFlyInstances(std::map<std::string, AbstractFlyInstance*> instances) {
    for (auto item : instances) {
        uint64_t lastPubTime = item.second->getLastPubTime();
        if (lastPubTime >= SENTINEL_PUBLISH_PERIOD + 1) {
            item.second->setLastPubTime(lastPubTime - SENTINEL_PUBLISH_PERIOD - 1);
        }
    }

    return;
}

bool FlyInstance::addSlave(const std::string &name, AbstractFlyInstance *slave) {
    if (this->slaves.find(name) != this->slaves.end()) {
        return false;
    }

    this->slaves[name] = slave;
    return true;
}

bool FlyInstance::addSentinel(const std::string &name, AbstractFlyInstance *sentinel) {
    if (this->sentinels.find(name) != this->sentinels.end()) {
        return false;
    }

    this->sentinels[name] = sentinel;
    return true;
}

uint64_t FlyInstance::getFailoverTimeout() const {
    return failoverTimeout;
}

void FlyInstance::setFailoverTimeout(uint64_t failoverTimeout) {
    this->failoverTimeout = failoverTimeout;
}

bool FlyInstance::startFailoverIfNeeded() {
    /** 如果没有处于客观下线状态，不允许执行failover操作 */
    if (0 == this->flags & FSI_O_DOWN) {
        return false;
    }

    /** 正在执行failover */
    if (0 == this->flags & FSI_FAILOVER_IN_PROGRESS) {
        return false;
    }

    /** 上次执行failover时间距离现在时间过短，则暂时先不执行failover */
    if (miscTool->mstime() - this->failoverStartTime < this->failoverTimeout * 2) {
        return false;
    }

    this->startFailover();
    return true;
}

/**
 * 终止failover
 * */
void FlyInstance::abortFailover() {
    assert(this->flags & FSI_FAILOVER_IN_PROGRESS);
    assert(this->failoverState < SENTINEL_FAILOVER_STATE_WAIT_PROMOTION);
    this->flags &= ~(FSI_FAILOVER_IN_PROGRESS | FSI_FORCE_FAILOVER);
    this->failoverState = SENTINEL_FAILOVER_STATE_NONE;
    this->failoverStateChangeTime = miscTool->mstime();
    if (NULL != this->promotedSlave) {
        this->promotedSlave->delFlags(FSI_PROMOTED);
        this->promotedSlave = NULL;
    }
}

void FlyInstance::startFailover() {
    /** must be master */
    assert(this->flags & FSI_MASTER);

    AbstractFlyServer *flySentinel = coordinator->getFlyServer();

    this->failoverState = SENTINEL_FAILOVER_STATE_WAIT_START;
    this->flags |= FSI_FAILOVER_IN_PROGRESS;
    this->failoverEpoch = flySentinel->getCurrentEpoch() + 1;
    flySentinel->sendEvent(LL_NOTICE, "+new-epoch", this, "%llu", flySentinel->getCurrentEpoch());
    flySentinel->sendEvent(LL_WARNING, "+try-failover", this, "%@");

    /** 添加一个随机时间，防止所有sentinel同时启动failover过程  */
    uint64_t nowt = miscTool->mstime();
    this->failoverStartTime = nowt + rand() % SENTINEL_MAX_DESYNC;
    this->failoverStateChangeTime = nowt;
}

void sentinelDiscardReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractInstanceLink *instanceLink = (AbstractInstanceLink *)context->data;
    if (NULL != instanceLink) {
        instanceLink->decreasePendingCommands();
    }
}

void sentinelPingReplyCallback(redisAsyncContext *context, void *reply, void *privdata) {
    AbstractFlyInstance *flyInstance = (AbstractFlyInstance *)privdata;
    AbstractInstanceLink *link = (AbstractInstanceLink *)context->data;
    redisReply *r = (redisReply*)reply;
    if (NULL == reply || NULL == link) {
        return;
    }

    /** 减少pending commands */
    link->decreasePendingCommands();

    uint64_t nowt = miscTool->mstime();
    if (REDIS_REPLY_STATUS == r->type
        || REDIS_REPLY_ERROR == r->type) {  /** 收到正确的回复 */
        if (0 == strncmp(r->str, "PONG", 4)
            || 0 == strncmp(r->str, "LOADING", 7)
            || 0 == strncmp(r->str, "MASTERDOWN", 10)) {
            link->setLastAvailTime(nowt);
            link->setActPingTime(0);
        } else {
            /**
             * 如果：
             *   1.收到失败的消息
             *   2.当前instance被标记为sdown
             *   3.没有发送过SCRIPT KILL命令
             * 则发送SCRIPT KILL命令
             **/
            if (0 == strncmp(r->str, "BUSY", 4)
                && (flyInstance->getFlags() & FSI_S_DOWN)
                && (0 == flyInstance->getFlags() & FSI_SCRIPT_KILL_SENT)) {
                if (redisAsyncCommand(flyInstance->getLink()->getCommandContext().get(),
                                      sentinelDiscardReplyCallback, flyInstance, "SCRIPT KILL")) {
                    flyInstance->getLink()->increasePendingCommands();
                }
                flyInstance->addFlags(FSI_SCRIPT_KILL_SENT);
            }
        }
    }

    /** 修改last pong时间为当前时间 */
    link->setLastPongTime(miscTool->mstime());
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
