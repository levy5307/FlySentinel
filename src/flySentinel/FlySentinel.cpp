//
// Created by levy on 2019/5/5.
//

#include <iostream>
#include <signal.h>
#include "FlySentinel.h"
#include "../flyClient/ClientDef.h"
#include "../flyClient/FlyClient.h"
#include "../def.h"
#include "../dataStructure/dict/Dict.cpp"
#include "FlysentinelDef.h"
#include "../scriptJob/ScriptJobDef.h"

FlySentinel::FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache)
        : AbstractFlyServer(coordinator, configCache) {
    memset(this->myid, 0, sizeof(this->myid));
    this->port = FLYDB_SENTINEL_PORT;
}

FlySentinel::~FlySentinel() {
    this->masters.clear();
    this->scriptsQueue.clear();
}

void FlySentinel::sendEvent(int level, char *type, std::shared_ptr<AbstractFlyDBInstance> flyInstance, const char *fmt, ...) {
    char msg[LOG_MAX_LEN];
    if ('%' == fmt[0] && '@' == fmt[1] && NULL != flyInstance) {
        if (flyInstance->haveMaster()) {
            std::shared_ptr<AbstractFlyDBInstance> master = flyInstance->getMaster();
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
        std::shared_ptr<AbstractFlyDBInstance> master = flyInstance->haveMaster() ? flyInstance->getMaster() : flyInstance;
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
void FlySentinel::callClientReconfScript(AbstractFlyDBInstance *master, int role, char *state,
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
