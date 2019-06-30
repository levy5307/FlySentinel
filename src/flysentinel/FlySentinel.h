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
#include "../scriptjob/ScriptJob.h"
#include "../coordinator/interface/AbstractInstanceLink.h"
#include "../commandtable/CommandTable.h"

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData);

class FlySentinel : public AbstractFlyServer {
public:
    /************************************************************************************************
     *******************                sentinel special interfaces                 *****************
     ************************************************************************************************/
    FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache);
    ~FlySentinel();
    void sendEvent(int level, char *type, AbstractFlyInstance *flyInstance, const char *fmt, ...);
    void generateInitMonitorEvents();
    int tryConnectionSharing(AbstractFlyInstance* flyInstance);
    void flushConfig();
    /** 将master的downAfterPeriod设置给与该master相连的所有sentinels和slaves */
    void propagateDownAfterPeriod(AbstractFlyInstance* master);
    void refreshInstanceInfo(AbstractFlyInstance* flyInstance, const std::string &info);
    const std::string &getAnnounceIP() const;
    void setAnnounceIP(const std::string &announceIP);
    int getAnnouncePort() const;
    void setAnnouncePort(int announcePort);
    const char *getMyid() const;
    uint64_t getCurrentEpoch() const;
    /** 处理从master/slave/sentinel发送来的hello message */
    void processHelloMessage(std::string &hello);
    void receiveHelloMessage(redisAsyncContext *context, void *reply, void *privdata);
    /** 通过pub/sub向flyInstance发送hello message，然后改flyInstance可以将该消息广播出去 */
    int sendHello(AbstractFlyInstance* flyInstance);
    bool sendPing(AbstractFlyInstance* flyInstance);
    void sendPeriodicCommands(AbstractFlyInstance* flyInstance);

    /************************************************************************************************
     *******************                general server interfaces                   *****************
     ************************************************************************************************/
    /** network */
    size_t getClientMaxQuerybufLen() const;
    int64_t getStatNetInputBytes() const;
    void addToStatNetInputBytes(int64_t size);
    const std::vector<int> &getIpfd() const;
    int getPort() const;
    void setPort(int port);

    /** clients */
    void addToClientsPendingToWrite(int fd);
    int handleClientsWithPendingWrites();
    std::shared_ptr<AbstractFlyClient> createClient(int fd);
    int freeClient(std::shared_ptr<AbstractFlyClient> flyClient);
    void freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient);
    void freeClientAsync(int fd);
    int getMaxClients() const;
    void unlinkClient(std::shared_ptr<AbstractFlyClient> flyClient);
    void linkClient(std::shared_ptr<AbstractFlyClient> flyClient);
    std::shared_ptr<AbstractFlyClient> getFlyClient(int fd);
    void freeClientsInAsyncFreeList();

    /** command */
    bool dealWithCommand(int fd);

    /** cron loop */
    int getHz() const;
    void setHz(int hz);
    time_t getNowt() const;
    void setNowt(time_t nowt);
    void addCronLoops();
    uint64_t getCronLoops() const;

    /** 将一组instance当前状态组成reply, 发送给flyClient */
    void addReplyRedisInstances(std::shared_ptr<AbstractFlyClient> flyClient,
                                std::map<std::string, AbstractFlyInstance*> instanceMap);

private:
    /************************************************************************************************
     *******************             sentinel server private functions              *****************
     ************************************************************************************************/
    void scheduleScriptExecution(char *path, ...);
    std::shared_ptr<ScriptJob> getScriptListNodeByPid(pid_t pid);
    void runPendingScripts();
    void collectTerminatedScripts();
    void deleteScriptJob(pid_t pid);
    void killTimedoutScripts();
    void callClientReconfScript(AbstractFlyInstance* master, int role,
                                char *state, SentinelAddr *from, SentinelAddr *to);
    AbstractFlyInstance* getFlyInstanceByAddrAndRunID(
            const std::map<std::string, AbstractFlyInstance*> &instances,
            const char *ip,
            int port,
            const char *runid);
    int updateSentinelAddrInAllMasters(AbstractFlyInstance* instance);
    AbstractFlyInstance* getMasterByName(const std::string &name);
    AbstractFlyInstance* getMasterByNameOrReplyError(
            std::shared_ptr<AbstractFlyClient> flyClient, std::string &str);
    void resetMaster(AbstractFlyInstance* master, int flags);
    int resetMasterByPattern(const std::string &pattern, int flags);
    void resetMasterAndChangeAddress(AbstractFlyInstance* master, const char *ip, int port);
    void setClientName(redisAsyncContext *context, AbstractFlyInstance* flyInstance, char *type);
    bool masterLookSane(AbstractFlyInstance* master);
    SentinelAddr* getCurrentMasterAddress(AbstractFlyInstance* master);
    void parseRunid(const std::string &line, AbstractFlyInstance* flyInstance);
    void parseSlaveIPAndPort(const std::string &line, AbstractFlyInstance *flyInstance);
    void parseSlaveRoleParams(const std::string &line, AbstractFlyInstance *flyInstance);

    /************************************************************************************************
     *******************             general server private functions               *****************
     ************************************************************************************************/
    void initGeneralServer(const AbstractCoordinator *coordinator, ConfigCache *configCache);
    void setMaxClientLimit();
    int listenToPort();
    void loadFromConfig(ConfigCache *configCache);
    void deleteFromPending(int fd);
    void deleteFromAsyncClose(int fd);

    /************************************************************************************************
     *******************                    Sentinel parameters                     *****************
     ************************************************************************************************/
    char myid[CONFIG_RUN_ID_SIZE + 1];
    uint64_t currentEpoch = 0;
    std::map<std::string, AbstractFlyInstance*> masters;            // key-name
    bool tilt = false;                 /** tilt mode */
    uint64_t tiltStartTime = 0;
    uint64_t previousTime = miscTool->mstime();
    std::string announceIP;                         /** 用于同其他sentinel进行gossip协议传输的地址和端口 */
    int announcePort = 0;
    int runningScripts = 0;
    std::list<std::shared_ptr<ScriptJob>> scriptsQueue;

    /************************************************************************************************
     *******************                General server parameters                  *****************
     ************************************************************************************************/
    /** 网络相关 */
    int port;                                 // tcp监听端口
    std::vector<int> ipfd;                    // TCP socket fd
    std::vector<std::string> bindAddrs;       // 绑定地址，监听地址
    int tcpBacklog;                           // TCP listen() backlog
    char *neterr;                             // 网络error buffer
    const char *unixsocket;                   // UNIX socket path
    mode_t unixsocketperm;                    // UNIX socket permission
    int usfd;                                 // Unix socket file descriptor
    int64_t statNetInputBytes;                // 该server从网络获取的byte数量
    int tcpKeepAlive;

    /** client相关 */
    int maxClients;                                                         /** 最大可同时连接的client数量 */
    uint64_t nextClientId;
    pthread_mutex_t nextClientIdMutex;
    std::map<int, std::shared_ptr<AbstractFlyClient> > clients;             /** client列表 */
    std::list<std::shared_ptr<AbstractFlyClient> > clientsPendingWrite;     /** 需要install write handler */
    std::list<std::shared_ptr<AbstractFlyClient> > clientsToClose;          /** 异步关闭的client链表 */
    uint64_t statRejectedConn;                                              /** 由于超过了maxclients而拒绝连接的次数 */
    size_t clientMaxQuerybufLen;                                            /** client buff最大长度 */
    uint64_t cronloops = 0;

    /** loop频率 */
    int hz;                                           // serverCron运行频率
    time_t nowt;                                      // 系统当前时间

    /** 命令表 */
    CommandTable* commandTable;

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_FLYSENTINEL_H
