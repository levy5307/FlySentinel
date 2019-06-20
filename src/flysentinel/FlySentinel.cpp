//
// Created by levy on 2019/5/5.
//

#include <iostream>
#include <signal.h>
#include <cassert>
#include "FlySentinel.h"
#include "../flyClient/ClientDef.h"
#include "../flyClient/FlyClient.h"
#include "../def.h"
#include "../dataStructure/dict/Dict.cpp"
#include "FlySentinelDef.h"
#include "../scriptJob/ScriptJobDef.h"
#include "../flyInstance/FlyInstanceDef.h"
#include "../flyInstance/FlyInstance.h"
#include "../atomic/AtomicHandler.h"

FlySentinel::FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache) {
    memset(this->myid, 0, sizeof(this->myid));
    initGeneralServer(coordinator, configCache);
}

void FlySentinel::initGeneralServer(const AbstractCoordinator *coordinator, ConfigCache *configCache) {
    this->coordinator = coordinator;

    // init command table
    this->commandTable = new CommandTable(coordinator);

    // 设置最大客户端数量
    setMaxClientLimit();

    // serverCron运行频率
    this->hz = CONFIG_CRON_HZ;
    this->neterr = new char[NET_ERR_LEN];

    // 拒绝连接次数设置为0
    this->statRejectedConn = 0;
    pthread_mutex_init(&this->nextClientIdMutex, NULL);

    // 当前时间
    this->nowt = time(NULL);
    this->clientMaxQuerybufLen = PROTO_MAX_QUERYBUF_LEN;

    // 从configCache获取参数
    this->loadFromConfig(configCache);

    // 打开监听socket，用于监听用户命令
    this->listenToPort();

    // 打开Unix domain socket
    if (NULL != this->unixsocket) {
        unlink(this->unixsocket);       // 如果存在，则删除unixsocket文件
        this->usfd = this->coordinator->getNetHandler()->unixServer(
                this->neterr,
                this->unixsocket,
                this->unixsocketperm,
                this->tcpBacklog);
        if (-1 == this->usfd) {
            std::cout << "Opening Unix Domain Socket: " << this->neterr << std::endl;
            exit(1);
        }
        this->coordinator->getNetHandler()->setBlock(NULL, this->usfd, 0);
    }

    this->logHandler = logFactory->getLogger();
}

FlySentinel::~FlySentinel() {
    this->masters.clear();
    this->scriptsQueue.clear();
}

/**
 * 发送事件，主要做了三件事：
 *   1.打印log
 *   2.发送pub/sub信息给相应的客户端，type是具体的消息
 *   3.生成通知脚本job到脚本job队列中
 **/
void FlySentinel::sendEvent(int level, char *type, std::shared_ptr<AbstractFlyInstance> flyInstance, const char *fmt, ...) {
    char msg[LOG_MAX_LEN];
    if ('%' == fmt[0] && '@' == fmt[1] && NULL != flyInstance) {
        if (flyInstance->haveMaster()) {
            std::shared_ptr<AbstractFlyInstance> master = flyInstance->getMaster();
            snprintf(msg, sizeof(msg), "%s %s %d @ %s %s %d",
                     flyInstance->getName().c_str(),
                     flyInstance->getAddr()->getIp().c_str(),
                     flyInstance->getAddr()->getPort(),
                     master->getName().c_str(),
                     master->getAddr()->getIp().c_str(),
                     master->getAddr()->getPort());
        } else {
            snprintf(msg, sizeof(msg), "%s %s %d",
                     flyInstance->getName().c_str(),
                     flyInstance->getAddr()->getIp().c_str(),
                     flyInstance->getAddr()->getPort());
        }
        fmt += 2;
    } else {
        msg[0] = '\0';
    }

    /** if fmt is not null */
    if (fmt[0] != '\0') {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(msg + strlen(msg), sizeof(msg) - strlen(msg), fmt, ap);
        va_end(ap);
    }

    /** log */
    coordinator->getLogHandler()->log(level, "%s %s", type, msg);

    /** 如果当前不是调试状态，则发送pub/sub消息 */
    if (level != LL_DEBUG) {
        coordinator->getPubSubHandler()->publishMessage(type, msg);
    }

    /** 调度脚本到脚本队列中 */
    if (LL_WARNING == level && NULL != flyInstance) {
        std::shared_ptr<AbstractFlyInstance> master = flyInstance->haveMaster() ? flyInstance->getMaster() : flyInstance;
        if (master && NULL != master->getNotificationScript()) {
            this->scheduleScriptExecution(master->getNotificationScript(), type, msg, NULL);
        }
    }
}

/**
 * 调用该函数有两个场景：
 *   1.这个函数值在启动时调用，用于向每个master发送一个+monitor事件
 *   2.同时在运行时动态监控master时，也会向该master发送一个+monitor事件
 **/
void FlySentinel::generateInitMonitorEvents() {
    for (auto item : this->masters) {
        this->sendEvent(LL_WARNING, "+monitor", item.second, "%@ quorum %d", item.second->getQuorum());
    }
}

/**
 * 该函数用于同一个sentinel的连接的不同master之间共享instancelink，参数必须是sentinel：
 *   具体步骤：
 *      1.对于所有的masters，轮流查看每一个master
 *      2.查看该master的sentinels中是否有一个实例与入参flyInstance代表相同的sentinel
 *      3.如果没有，继续下一个master；如果有，则与该master共享instance link
 **/
int FlySentinel::tryConnectionSharing(std::shared_ptr<AbstractFlyInstance> flyInstance) {
    /** flyInstance必须是sentinel */
    assert(flyInstance->getFlags() & FSI_SENTINEL);

    /** 该instance的runid为空 */
    if (flyInstance->getRunid().empty()) {
        return -1;
    }

    /** 该instance link已经被共享过 */
    if (flyInstance->getLink().use_count() > 1) {
        return -1;
    }

    /** 遍历所有的masters */
    for (auto item : this->masters) {
        std::shared_ptr<AbstractFlyInstance> master = item.second;

        /** 如果当前遍历的master是flyInstance的master，直接跳过 */
        if (flyInstance->getMaster() == master) {
            continue;
        }

        /** 从当前master中的所有sentinel中获取是否有与flyInstance代表同一sentinel的结构-->match */
        std::shared_ptr<AbstractFlyInstance> match = getFlyInstanceByAddrAndRunID(
                master->getSentinels(), NULL, 0, flyInstance->getRunid().c_str());
        /** 没有找到则继续从下一个master的sentinel中查找 */
        if (NULL == match || flyInstance == match) {
            continue;
        }

        /** 找到了，则共享instance link */
        flyInstance->releaseLink();
        flyInstance->setLink(match->getLink());
        return 1;
    }

    return -1;
}

/** 可变参数列表需要以NULL结尾 */
void FlySentinel::scheduleScriptExecution(char *path, ...) {
    int argc = 1;
    char *argv[SENTINEL_SCRIPT_MAX_ARGS + 1];
    va_list ap;

    /** 解析参数, 可变长度参数以NULL结尾 */
    argv[0] = (char*)new std::string(path);
    va_start(ap, path);
    while (argc < SENTINEL_SCRIPT_MAX_ARGS) {
        char *arg = va_arg(ap, char*);
        if (NULL == arg) {
            break;
        }
        argv[argc++] = (char*)new std::string(arg);
    }
    va_end(ap);

    /** 生成一个job并存入job队列 */
    std::shared_ptr<ScriptJob> job = std::shared_ptr<ScriptJob>(new ScriptJob(argc, argv));
    this->scriptsQueue.push_back(job);
    /** 如果队列长度超过最大限度，则删除列首的一个没在执行的job */
    if (this->scriptsQueue.size() > SENTINEL_SCRIPT_MAX_QUEUE) {
        std::list<std::shared_ptr<ScriptJob>>::iterator iter = this->scriptsQueue.begin();
        for (; iter != this->scriptsQueue.end(); iter++) {
            if ((*iter)->isRunning()) { /** 如果该job处于运行状态，则寻找下一个 */
                continue;
            }
            this->scriptsQueue.erase(iter);
            break;
        }
    }
}

std::shared_ptr<ScriptJob> FlySentinel::getScriptListNodeByPid(pid_t pid) {
    std::list<std::shared_ptr<ScriptJob>>::iterator iter = this->scriptsQueue.begin();
    for (; iter != this->scriptsQueue.end(); iter++) {
        if ((*iter)->isRunning() && (*iter)->getPid() == pid) {
            return *iter;
        }
    }

    return NULL;
}

void FlySentinel::runPendingScripts() {
    /** 获取当前时间 */
    uint64_t nowt = miscTool->mstime();

    for (auto item : this->scriptsQueue) {
        if (this->runningScripts >= SENTINEL_SCRIPT_MAX_RUNNING) {
            return;
        }

        /** 该job处于运行状态 */
        if (item->isRunning()) {
            continue;
        }

        /** 如果是重试，并且还没到下次重试时间 */
        if (item->getStartTime() > nowt) {
            continue;
        }

        /** set job parameters */
        item->addFlags(SENTINEL_SCRIPT_RUNNING);
        item->setStartTime(nowt);
        item->addRetryCount();

        /** create child process to deal with script job */
        pid_t pid = fork();
        if (-1 == pid) {
            /** error */
            this->sendEvent(LL_WARNING, "-script-error", NULL, "%s %d %d", item->getArgv()[0], 99, 0);
            item->delFlags(SENTINEL_SCRIPT_RUNNING);
            item->setPid(0);
        } else if (0 == pid) {
            /** child */
            execve(item->getArgv()[0], item->getArgv(), NULL);
            exit(2);
        } else {
            /** parent */
            this->runningScripts++;
            item->setPid(pid);
            this->sendEvent(LL_DEBUG, "+script-child", NULL, "%ld", (long)pid);
        }
    }
}

/**
 * 查看运行完的scripts
 *  1.对于成功结束的，从队列中移除
 *  2.对于信号终止或者exit=1的，则调度其继续运行，直到达到最大重试次数(SENTINEL_SCRIPT_MAX_RETRY)
 **/
void FlySentinel::collectTerminatedScripts() {
    int statloc = 0;
    pid_t pid = 0;
    while (pid = wait3(&statloc, WNOHANG, NULL) > 0) {
        int exitcode = WEXITSTATUS(statloc);
        int bysignal = 0;
        if (WIFSIGNALED(statloc)) {
            bysignal = WTERMSIG(statloc);
        }

        /** get scriptjob by pid */
        std::shared_ptr<ScriptJob> job = this->getScriptListNodeByPid(pid);
        if (NULL == job) {
            coordinator->getLogHandler()->logWarning("wait3() returned a pid (%ld) we can't find in our scripts execution queue!", (long)pid);
            continue;
        }

        /** 重新调度该任务运行 */
        if ((0 != bysignal || 1 == exitcode) && job->getRetryCount() < SENTINEL_SCRIPT_MAX_RETRY) {
            job->reschedule();
        } else { /** 将job从队列中移除 */
            /** 如果非正常终止，则打印log */
            if (0 != bysignal || 0 != exitcode) {
                coordinator->getLogHandler()->logWarning("-script-error %s %d %d", job->getArgv()[0], bysignal, exitcode);
            }

            /** delete job from queue */
            this->deleteScriptJob(pid);
        }
    }
}

/** 杀死超时的job，这些被杀死的job后续在collectTerminatedScripts中将会被处理 */
void FlySentinel::killTimedoutScripts() {
    uint64_t nowt = miscTool->mstime();

    /** 遍历脚本任务列表，删除过时的job */
    for (auto item : this->scriptsQueue) {
        if (item->isRunning() && nowt - item->getStartTime() > SENTINEL_SCRIPT_MAX_RUNTIME) {
            coordinator->getLogHandler()->logWarning("-script-timeout", NULL, "%s %ld", item->getArgv()[0], (long)item->getPid());
            kill(item->getPid(), SIGKILL);
        }
    }
}

/**
 * 客户端重配置脚本job, 每次failover执行时都会被调用：
 *   <master-name> <role> <state> <from-ip> <from-port> <to-ip> <to-port>
 *   <state>始终是"failover"
 *   <role>是"leader"或者"observer"
 * */
void FlySentinel::callClientReconfScript(AbstractFlyInstance *master, int role, char *state,
                                         SentinelAddr *from, SentinelAddr *to) {
    if (master->isClientReconfigScriptNULL()) {
        return;
    }

    /** 获取port的字符串 */
    std::string fromport = std::to_string(from->getPort());
    std::string toport = std::to_string(to->getPort());

    /** 调度任务进任务队列 */
    this->scheduleScriptExecution(master->getClientReconfigScript(),
                                  master->getName().c_str(),
                                  SENTINEL_OBSERVER == role ? "leader" : "observer",
                                  state,
                                  from->getIp().c_str(),
                                  fromport.c_str(),
                                  to->getIp().c_str(),
                                  toport.c_str(),
                                  NULL);

}


std::shared_ptr<AbstractFlyInstance> FlySentinel::getFlyInstanceByAddrAndRunID(const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &instances,
                                                                               const char *ip,
                                                                               int port,
                                                                               const char *runid) {
    /** 两者都为NULL那还找什么找?! 看函数名字！*/
    if (NULL == ip && NULL == runid) {
        return NULL;
    }

    /** 遍历所有的instances */
    for (auto item : instances) {
        std::shared_ptr<AbstractFlyInstance> instance = item.second;
        if ((NULL == runid || 0 == instance->getRunid().compare(runid))
            && (NULL == ip || 0 == instance->getAddr()->getIp().compare(ip))
            && port == instance->getAddr()->getPort()) {
            return instance;
        }
    }

    /** 没有找到，返回NULL */
    return NULL;
}

int FlySentinel::updateSentinelAddrInAllMasters(std::shared_ptr<AbstractFlyInstance> flyInstance) {
    /** 必须是sentinel */
    assert(flyInstance->getFlags() & FSI_SENTINEL);

    /** 记录重配置的数量 */
    int reconfigured = 0;

    /** 遍历所有的master */
    for (auto item : this->masters) {
        std::shared_ptr<AbstractFlyInstance> master = item.second;

        /** 从master的sentinels中找到与instance代表同一个sentinel的flyInstance, 如果找不到，则继续处理下一个master */
        std::shared_ptr<AbstractFlyInstance> match =
                getFlyInstanceByAddrAndRunID(master->getSentinels(), NULL, 0, flyInstance->getRunid().c_str());
        if (NULL == match) {
            continue;
        }

        /** 因为修改地址，旧的连接不在有效，所以断开旧的连接 */
        if (NULL != match->getLink()->getCommandContext()) {
            match->getLink()->closeConnection(match->getLink()->getCommandContext());
        }
        if (NULL != match->getLink()->getPubsubContext()) {
            match->getLink()->closeConnection(match->getLink()->getPubsubContext());
        }

        /** 如果找到的instance刚好是自己，则无需进行dup address操作 */
        if (match == flyInstance) {
            continue;
        }

        match->dupAddr(flyInstance->getAddr());
        reconfigured++;
    }

    /** 如果进行了重配置，则要发送事件 */
    if (reconfigured > 0) {
        sendEvent(LL_NOTICE, "+sentinel-address-update", flyInstance, "%@ %d additional matching instances", reconfigured);
    }

    return reconfigured;
}

std::shared_ptr<AbstractFlyInstance> FlySentinel::getMasterByName(char *name) {
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>>::iterator iter = this->masters.find(name);
    if (iter != this->masters.end()) {
        return iter->second;
    }

    return NULL;
}

void FlySentinel::resetMaster(std::shared_ptr<AbstractFlyInstance> master, int flags) {
    assert(master->getFlags() & FSI_MASTER);
    master->reset(flags);
    if (flags & SENTINEL_GENERATE_EVENT) {
        this->sendEvent(LL_WARNING, "+reset-master", master, "%@");
    }
}

int FlySentinel::resetMasterByPattern(const std::string &pattern, int flags) {
    int reset = 0;
    for (auto item : this->masters) {
        if (miscTool->stringmatch(pattern, item.first, false)) {
            resetMaster(item.second, flags);
            reset++;
        }
    }

    return reset;
}

/** reset master and change it`s address */
void FlySentinel::resetMasterAndChangeAddress(std::shared_ptr<AbstractFlyInstance> master, char *ip, int port) {
    SentinelAddr *newaddr = new SentinelAddr(ip, port);
    SentinelAddr *oldaddr = master->getAddr();

    /** 收集所有slave的地址（被切换成master的地址除外）*/
    const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> slaves = master->getSlaves();
    std::vector<SentinelAddr*> slaveAddrs;
    for (auto item : slaves) {
        if (*(item.second->getAddr()) == *newaddr) {
            continue;
        }

        slaveAddrs.push_back(item.second->getAddr());
    }

    /** 如果master的切换后地址和原旧地址不同，也将旧地址加入slaveAddrs中 */
    if (*newaddr != *oldaddr) {
        slaveAddrs.push_back(oldaddr);
    }

    /** reset master */
    this->resetMaster(master, SENTINEL_RESET_SENTINELS);
    master->setAddr(newaddr);

    /** add slaves back */
    for (auto item : slaveAddrs) {
        std::shared_ptr<AbstractFlyInstance> slave = std::shared_ptr<AbstractFlyInstance>(
                new FlyInstance(NULL, FSI_SLAVE, item->getIp(), item->getPort(), master->getQuorum(), master));
        this->sendEvent(LL_NOTICE, "+slave", slave, "%@");
    }
    slaveAddrs.clear();

    /** flush config */
    this->flushConfig();
}

void FlySentinel::flushConfig() {

}

/** 将master的downAfterPeriod设置给与该master相连的所有sentinels和slaves */
void FlySentinel::propagateDownAfterPeriod(std::shared_ptr<AbstractFlyInstance> master) {
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>> slaves = master->getSlaves();
    for (auto item : slaves) {
        item.second->setDownAfterPeriod(master->getDownAfterPeriod());
    }

    std::map<std::string, std::shared_ptr<AbstractFlyInstance>> sentinels = master->getSentinels();
    for (auto item : sentinels) {
        item.second->setDownAfterPeriod(master->getDownAfterPeriod());
    }
}

void FlySentinel::setClientName(redisAsyncContext *context, std::shared_ptr<AbstractFlyInstance> flyInstance, char *type) {
    char name[64];
    snprintf(name, sizeof(name), "sentinel-%.8s-%s", this->myid, type);
    if (redisAsyncCommand(context, sentinelDiscardReplyCallback, flyInstance.get(), "%s SETNAME %s", "CLIENT", name)) {
        flyInstance->getLink()->increasePendingCommands();
    }
}

/**
 * 鉴定master是否是正常的，如果master正常，需要满足以下条件：
 *    1.master在配置中是master
 *    2.master在自己的报告中也是master
 *    3.没有处于SDOWN和ODOWN
 *    4.master在[nowt-2*SENTINEL_INFO_PERIOD, nowt]时间内接收过info信息
 **/
bool FlySentinel::masterLookSane(std::shared_ptr<AbstractFlyInstance> master) {
    return (master->getFlags() & FSI_MASTER)
           && (master->getRoleReported() & FSI_MASTER)
           && (0 == master->getFlags() & (FSI_O_DOWN | FSI_S_DOWN))
           && (master->getInfoRefresh() > miscTool->mstime() - 2 * SENTINEL_INFO_PERIOD);
}

void FlySentinel::deleteScriptJob(pid_t pid) {
    std::list<std::shared_ptr<ScriptJob>>::iterator iter = this->scriptsQueue.begin();
    for (iter; iter != this->scriptsQueue.end(); iter++) {
        if ((*iter)->getPid() == pid) {
            this->scriptsQueue.erase(iter);
            break;
        }
    }

    this->runningScripts--;
}

SentinelAddr* FlySentinel::getCurrentMasterAddress(std::shared_ptr<AbstractFlyInstance> master) {
    if ((master->getFlags() & FSI_FAILOVER_IN_PROGRESS)
        && master->hasPromotedSlave()
        && master->getFailoverState() > SENTINEL_FAILOVER_STATE_RECONF_SLAVES) {
        return master->getPromotedSlave()->getAddr();
    } else {
        master->getAddr();
    }
}

void FlySentinel::refreshInstanceInfo(AbstractFlyInstance* flyInstance, const std::string &info) {
}

void FlySentinel::addToClientsPendingToWrite(int fd) {
    std::shared_ptr<AbstractFlyClient> flyClient = this->getFlyClient(fd);
    /** 如果没有找到，返回 */
    if (nullptr == flyClient) {
        return;
    }

    /**
     * 只有处于初始状态或者已连接状态的client才会加入写入队列，
     * 其他情况则不加入，代表只会将数据写入输出缓冲区，并不会真正发送
     **/
    if ((0 == (flyClient->getFlags() & CLIENT_PENDING_WRITE))) {
        flyClient->addFlag(CLIENT_PENDING_WRITE);
        this->clientsPendingWrite.push_back(flyClient);
    }
}

int FlySentinel::handleClientsWithPendingWrites() {
    int pendingCount = this->clientsPendingWrite.size();
    if (0 == pendingCount) {
        return 0;
    }

    std::list<std::shared_ptr<AbstractFlyClient> >::iterator iter = this->clientsPendingWrite.begin();
    for (; iter != this->clientsPendingWrite.end(); iter++) {
        // 先清除标记，清空了该标记才回保证该客户端再次加入到clientsPendingWrite里；
        // 否则无法加入。也就无法处理其输出
        (*iter)->delFlag(CLIENT_PENDING_WRITE);

        // 先直接发送，如果发送不完，再创建文件事件异步发送
        if (-1 == (*iter)->writeToClient(0)) {
            continue;
        }

        // 异步发送
        if (!(*iter)->hasNoPending()) {
            if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                    (*iter)->getFd(), ES_WRITABLE, sendReplyToClient, NULL)) {
                freeClientAsync(*iter);
            }
        }
    }

    /** 清空列表 */
    this->clientsPendingWrite.clear();

    /** 返回清空的client数量 */
    return pendingCount;
}

void FlySentinel::freeClientAsync(int fd) {
    std::shared_ptr<AbstractFlyClient> flyClient = this->getFlyClient(fd);
    if (nullptr == flyClient) {
        return;
    }

    freeClientAsync(flyClient);
}

std::shared_ptr<AbstractFlyClient> FlySentinel::createClient(int fd) {
    if (fd <= 0) {
        return nullptr;
    }

    // 超过了客户端最大数量
    if (this->clients.size() >= this->maxClients) {
        this->statRejectedConn++;
        return nullptr;
    }

    // create FlyClient
    std::shared_ptr<AbstractFlyClient> flyClient = this->coordinator
            ->getFlyClientFactory()
            ->getFlyClient(fd, this->nowt);
    uint64_t clientId = 0;
    atomicGetIncr(this->nextClientId, clientId, 1);
    flyClient->setId(clientId);

    // 设置读socket，并为其创建相应的file event
    this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
    this->coordinator->getNetHandler()->enableTcpNoDelay(NULL, fd);
    if (this->tcpKeepAlive > 0) {
        this->coordinator->getNetHandler()->keepAlive(
                NULL, fd, this->tcpKeepAlive);
    }
    if (-1 == this->coordinator->getEventLoop()->createFileEvent(
            fd, ES_READABLE, readQueryFromClient, NULL)) {
        return nullptr;
    }

    // 加入到clients队列中
    this->clients[fd] = flyClient;

    return flyClient;
}

int FlySentinel::freeClient(std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 将其从global list中删除*/
    this->unlinkClient(flyClient);
}

void FlySentinel::freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient) {
    if (flyClient->getFlags() & CLIENT_CLOSE_ASAP) {
        return;
    }

    flyClient->setFlags(CLIENT_CLOSE_ASAP);
    this->clientsToClose.push_back(flyClient);
}

/**
 * 将client从一切global list中删除掉
 * (除async delete列表之外, 否则可能导致无法删除)
 **/
void FlySentinel::unlinkClient(std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 在clients列表中删除，并删除该client对应的文件事件 */
    int fd = flyClient->getFd();
    if (-1 != fd) {
        if (this->clients.find(fd) != this->clients.end()) {
            coordinator->getEventLoop()->deleteFileEvent(flyClient->getFd(), ES_WRITABLE | ES_READABLE);
            close(flyClient->getFd());
            flyClient->setFd(-1);
            this->clients.erase(flyClient->getFd());
        }
    }

    /** 将其从pending write列表中删除 */
    if (flyClient->IsPendingWrite()) {
        this->deleteFromPending(flyClient->getFd());
    }

    /** 在async close列表中删除 */
    this->deleteFromAsyncClose(flyClient->getFd());

}

void FlySentinel::deleteFromPending(int fd) {
    std::list<std::shared_ptr<AbstractFlyClient> >::iterator iter = this->clientsPendingWrite.begin();
    for (; iter != this->clientsPendingWrite.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsPendingWrite.erase(iter);
            return;
        }
    }
}

void FlySentinel::deleteFromAsyncClose(int fd) {
    std::list<std::shared_ptr<AbstractFlyClient> >::iterator iter = this->clientsToClose.begin();
    for (iter; iter != this->clientsToClose.end(); iter++) {
        if ((*iter)->getFd() == fd) {
            this->clientsToClose.erase(iter);
            return;
        }
    }
}

void FlySentinel::linkClient(std::shared_ptr<AbstractFlyClient> flyClient) {
    this->clients[flyClient->getFd()] = flyClient;
}

std::shared_ptr<AbstractFlyClient> FlySentinel::getFlyClient(int fd) {
    if (this->clients.find(fd) != this->clients.end()) {
        return this->clients[fd];
    }

    return nullptr;
}

void FlySentinel::freeClientsInAsyncFreeList() {
    for (auto client : this->clientsToClose) {
        if (nullptr == client) {
            continue;
        }
        freeClient(client);
    }

    this->clientsToClose.clear();
}

int FlySentinel::getMaxClients() const {
    return this->maxClients;
}

int FlySentinel::listenToPort() {
    int fd;
    // try to bind all to IPV4 and IPV6
    if (0 == this->bindAddrs.size()) {
        int success = 0;
        // try to set *(any address) to ipv6
        fd = this->coordinator->getNetHandler()->tcp6Server(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        // try to set *(any address) to ipv4
        fd = this->coordinator->getNetHandler()->tcpServer(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        if (0 == success) {
            return -1;
        }
    } else {
        for (auto addr : this->bindAddrs) {
            // 如果是IPV6
            if (addr.find(":") != addr.npos) {
                fd = this->coordinator->getNetHandler()->tcp6Server(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            } else {
                fd = this->coordinator->getNetHandler()->tcpServer(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            }
            if (-1 == fd) {
                return -1;
            }
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
        }
    }

    return 1;
}

void FlySentinel::loadFromConfig(ConfigCache *configCache) {
    this->bindAddrs = configCache->getBindAddrs();
    this->unixsocket = configCache->getUnixsocket();
    this->unixsocketperm = configCache->getUnixsocketperm();
    this->tcpKeepAlive = configCache->getTcpKeepAlive();
    this->port = configCache->getPort();
}

bool FlySentinel::dealWithCommand(int fd) {
    std::shared_ptr<AbstractFlyClient> flyClient = this->getFlyClient(fd);
    if (nullptr == flyClient) {
        return false;
    }

    return this->commandTable->dealWithCommand(flyClient);
}

void FlySentinel::setMaxClientLimit() {
    this->maxClients = CONFIG_DEFAULT_MAX_CLIENTS;
    int maxFiles = this->maxClients + CONFIG_MIN_RESERVED_FDS;
    rlimit limit;

    // 获取当前进程可打开的最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        // 如果获取失败, 按照进程中最大文件数量为1024计算(内核默认1024),
        // 重置maxClients
        this->maxClients = 1024 - CONFIG_MIN_RESERVED_FDS;
    } else {
        int softLimit = limit.rlim_cur;
        // 如果soft limit小于maxfiles, 则尝试增大soft limit, 并重置maxClients
        if (softLimit < maxFiles) {
            int curLimit = maxFiles;
            int decrStep = 16;

            // 逐步试探提高softlimit
            while (curLimit > softLimit) {
                limit.rlim_cur = curLimit;
                if (setrlimit(RLIMIT_NOFILE, &limit) != -1) {
                    break;
                }
                curLimit -= decrStep;
            }
            if (curLimit < softLimit) {
                curLimit = softLimit;
            }

            // 如果当前文件数量限制小于最小保留文件数，程序退出
            if (curLimit <= CONFIG_MIN_RESERVED_FDS) {
                exit(1);
            }

            this->maxClients = curLimit - CONFIG_MIN_RESERVED_FDS;
        }
    }
}

int FlySentinel::getHz() const {
    return hz;
}

void FlySentinel::setHz(int hz) {
    this->hz = hz;
}

time_t FlySentinel::getNowt() const {
    return nowt;
}

void FlySentinel::setNowt(time_t nowt) {
    this->nowt = nowt;
}

size_t FlySentinel::getClientMaxQuerybufLen() const {
    return clientMaxQuerybufLen;
}

int64_t FlySentinel::getStatNetInputBytes() const {
    return statNetInputBytes;
}

void FlySentinel::addToStatNetInputBytes(int64_t size) {
    this->clientMaxQuerybufLen += size;
}

void FlySentinel::addCronLoops() {
    this->cronloops++;
}

uint64_t FlySentinel::getCronLoops() const {
    return this->cronloops;
}

const std::vector<int> &FlySentinel::getIpfd() const {
    return ipfd;
}

const std::string &FlySentinel::getAnnounceIP() const {
    return announceIP;
}

void FlySentinel::setAnnounceIP(const std::string &announceIP) {
    this->announceIP = announceIP;
}

int FlySentinel::getAnnouncePort() const {
    return announcePort;
}

void FlySentinel::setAnnouncePort(int announcePort) {
    this->announcePort = announcePort;
}

int FlySentinel::getPort() const {
    return port;
}

void FlySentinel::setPort(int port) {
    this->port = port;
}

const char *FlySentinel::getMyid() const {
    return myid;
}

uint64_t FlySentinel::getCurrentEpoch() const {
    return currentEpoch;
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

