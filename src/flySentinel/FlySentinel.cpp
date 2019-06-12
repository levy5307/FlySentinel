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

FlySentinel::FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache)
        : AbstractFlyServer(coordinator, configCache) {
    memset(this->myid, 0, sizeof(this->myid));
    this->port = FLYDB_SENTINEL_PORT;
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
    this->logHandler->log(level, "%s %s", type, msg);

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
            logHandler->logWarning("wait3() returned a pid (%ld) we can't find in our scripts execution queue!", (long)pid);
            continue;
        }

        /** 重新调度该任务运行 */
        if ((0 != bysignal || 1 == exitcode) && job->getRetryCount() < SENTINEL_SCRIPT_MAX_RETRY) {
            job->reschedule();
        } else { /** 将job从队列中移除 */
            /** 如果非正常终止，则打印log */
            if (0 != bysignal || 0 != exitcode) {
                logHandler->logWarning("-script-error %s %d %d", job->getArgv()[0], bysignal, exitcode);
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
            logHandler->logWarning("-script-timeout", NULL, "%s %ld", item->getArgv()[0], (long)item->getPid());
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

