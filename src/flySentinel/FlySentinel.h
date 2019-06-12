//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYSENTINEL_H
#define FLYSENTINEL_FLYSENTINEL_H

#include <map>
#include <vector>
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../config/ConfigCache.h"
#include "../def.h"
#include "../coordinator/interface/AbstractFlyInstance.h"
#include "../scriptJob/ScriptJob.h"
#include "../coordinator/interface/AbstractInstanceLink.h"

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData);

class FlySentinel : public BaseFlyServer {
public:
    FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache);
    ~FlySentinel();
    void sendEvent(int level, char *type, std::shared_ptr<AbstractFlyInstance> flyInstance, const char *fmt, ...);
    void generateInitMonitorEvents();
    int tryConnectionSharing(std::shared_ptr<AbstractFlyInstance> flyInstance);
    void flushConfig();
    /** 将master的downAfterPeriod设置给与该master相连的所有sentinels和slaves */
    void propagateDownAfterPeriod(std::shared_ptr<AbstractFlyInstance> master);
    void refreshInstanceInfo(AbstractFlyInstance* flyInstance, const std::string &info);

private:
    void scheduleScriptExecution(char *path, ...);
    std::shared_ptr<ScriptJob> getScriptListNodeByPid(pid_t pid);
    void runPendingScripts();
    void collectTerminatedScripts();
    void deleteScriptJob(pid_t pid);
    void killTimedoutScripts();
    void callClientReconfScript(AbstractFlyInstance *master, int role, char *state, SentinelAddr *from, SentinelAddr *to);
    std::shared_ptr<AbstractFlyInstance> getFlyInstanceByAddrAndRunID(
            const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &instances,
            const char *ip,
            int port,
            const char *runid);
    int updateSentinelAddrInAllMasters(std::shared_ptr<AbstractFlyInstance> instance);
    std::shared_ptr<AbstractFlyInstance> getMasterByName(char *name);
    void resetMaster(std::shared_ptr<AbstractFlyInstance> master, int flags);
    int resetMasterByPattern(const std::string &pattern, int flags);
    void resetMasterAndChangeAddress(std::shared_ptr<AbstractFlyInstance> master, char *ip, int port);
    void setClientName(redisAsyncContext *context, std::shared_ptr<AbstractFlyInstance> flyInstance, char *type);
    bool masterLookSane(std::shared_ptr<AbstractFlyInstance> master);
    SentinelAddr* getCurrentMasterAddress(std::shared_ptr<AbstractFlyInstance> master);

    char myid[CONFIG_RUN_ID_SIZE + 1];
    uint64_t currentEpoch = 0;
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>> masters;            // key-name
    bool tilt = false;                 /** tilt mode */
    uint64_t tiltStartTime = 0;
    uint64_t previousTime = miscTool->mstime();
    char *announceIP = NULL;
    int announcePort = 0;
    int runningScripts = 0;
    std::list<std::shared_ptr<ScriptJob>> scriptsQueue;
};


#endif //FLYSENTINEL_FLYSENTINEL_H
