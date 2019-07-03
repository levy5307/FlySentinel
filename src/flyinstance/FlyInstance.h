//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYINSTANCE_H
#define FLYSENTINEL_FLYINSTANCE_H

#include "../coordinator/interface/AbstractFlyInstance.h"
#include "../def.h"
#include "FlyInstanceDef.h"

void sentinelDiscardReplyCallback(redisAsyncContext *context, void *reply, void *privdata);
void sentinelInfoReplyCallback(redisAsyncContext *context, void *reply, void *privdata);
void sentinelPublishReplyCallback(redisAsyncContext *context, void *reply, void *privdata);
void sentinelPingReplyCallback(redisAsyncContext *context, void *reply, void *privdata);

class FlyInstance : public AbstractFlyInstance {
public:
    FlyInstance(const std::string &name, int flags, const std::string &hostname,
                int port, uint32_t quorum, AbstractFlyInstance* master);
    ~FlyInstance();
    int getFlags() const;
    void setFlags(int flags);
    const std::string &getName() const;
    void setName(const std::string &name);
    SentinelAddr *getAddr() const;
    void setAddr(SentinelAddr *addr);
    void dupAddr(SentinelAddr *addr);
    void setPort(int port);
    AbstractFlyInstance* getMaster() const;
    bool haveMaster() const;
    void setMaster(AbstractFlyInstance* master);
    uint32_t getQuorum() const;
    void setQuorum(uint32_t quorum);
    char *getNotificationScript() const;
    void setNotificationScript(char *notificationScript);
    char *getClientReconfigScript() const;
    bool isClientReconfigScriptNULL() const;
    const std::shared_ptr<AbstractInstanceLink> &getLink() const;
    void setLink(const std::shared_ptr<AbstractInstanceLink> &link);
    void releaseLink();
    const std::string &getRunid() const;
    void setRunid(const std::string &runid);
    const std::map<std::string, AbstractFlyInstance*> &getSentinels() const;
    const std::map<std::string, AbstractFlyInstance*> &getSlaves() const;
    AbstractFlyInstance* lookupSlave(char *ip, int port);
    int removeMatchingSentinel(const std::string &runid);
    void reset(int flags);
    bool noDownFor(uint64_t ms);
    uint64_t getDownAfterPeriod() const;
    void setDownAfterPeriod(uint64_t downAfterPeriod);
    int getRoleReported() const;
    void setRoleReported(int roleReported);
    uint64_t getRoleReportedTime() const;
    void setRoleReportedTime(uint64_t roleReportedTime);
    uint64_t getInfoRefresh() const;
    void setInfoRefresh(uint64_t infoRefresh);
    AbstractFlyInstance* getPromotedSlave() const;
    void setPromotedSlave(AbstractFlyInstance* promotedSlave);
    bool hasPromotedSlave() const;
    FailoverState getFailoverState() const;
    void setFailoverState(FailoverState failoverState);
    void clearInfo();
    void setInfo(const std::string &info);
    const std::string& getInfo() const;
    uint64_t getConfigEpoch() const;
    void setConfigEpoch(uint64_t configEpoch);
    uint64_t getLastPubTime() const;
    void setLastPubTime(uint64_t lastPubTime);
    uint64_t getLastHelloTime() const;
    void setLastHelloTime(uint64_t lastHelloTime);
    void addReplySentinelRedisInstance(std::shared_ptr<AbstractFlyClient> flyClient);
    int sentinelIsQuorumReachable(int *usablePtr);
    uint64_t getMasterLinkDownTime() const;
    void setMasterLinkDownTime(uint64_t masterLinkDownTime);
    const std::string &getSlaveMasterHost() const;
    void setSlaveMasterHost(const std::string &slaveMasterHost);
    int getSlaveMasterPort() const;
    void setSlaveMasterPort(int slaveMasterPort);
    int getSlaveMasterLinkStatus() const;
    void setSlaveMasterLinkStatus(int slaveMasterLinkStatus);
    int getSlavePriority() const;
    void setSlavePriority(int slavePriority);
    uint64_t getSlaveReplOffset() const;
    void setSlaveReplOffset(uint64_t slaveReplOffset);
    uint64_t getSlaveConfChangeTime() const;
    void setSlaveConfChangeTime(uint64_t slaveConfChangeTime);
    bool isPromotedSlave();

private:
    FlyInstance(){};
    std::string getFlagsString();
    int flags;
    std::string name;
    std::string runid;
    uint64_t configEpoch;           /** 配置epoch */
    uint64_t lastPubTime;         /** 最后一次向Pub/Sub发送hello的时间 */
    uint64_t lastHelloTime;       /** 只有当FSI_SENTINEL被设置时才会使用，表示最近一次从Pub/Sub接收到hello消息的时间 */
    SentinelAddr *addr = NULL;
    AbstractFlyInstance* master = NULL;
    char *notificationScript = NULL;
    char *clientReconfigScript = NULL;
    std::shared_ptr<AbstractInstanceLink> link;     /** 与instance的连接，sentinel之间共享 */

    /**
     * master参数
     **/
    std::map<std::string, AbstractFlyInstance*> sentinels;    /** key-ip:port. Other sentinels monitoring the same master. */
    std::map<std::string, AbstractFlyInstance*> slaves;       /** key-ip:port. Slaves for this master instance. */
    uint32_t quorum;                                          /** 对于判定flydb失败，需要的sentinel票数 */

    uint64_t sDownSinceTime = 0;                        /** Subjectively down since time. */
    uint64_t oDownSinceTime = 0;                        /** Objectively down since time. */
    uint64_t downAfterPeriod = 0;                       /** down after this period */
    uint64_t masterLinkDownTime;                        /** Slave replication link down time. */
    AbstractFlyInstance* promotedSlave = NULL;
    FailoverState failoverState = SENTINEL_FAILOVER_STATE_NONE;
    int roleReported;
    uint64_t roleReportedTime = 0;
    uint64_t infoRefresh = 0;
    std::string info;                                   /** 缓存的输出info */

    /**
     * slave参数
     **/
    std::string slaveMasterHost;                      /** 通过info信息获取到的master host */
    int slaveMasterPort;                              /** 通过info信息获取到的master port */
    int slavePriority;                                /** 通过info信息获取到的slave的优先级 */
    uint64_t slaveReplOffset;                         /** 通过info信息获取到的slave的主从复制偏移量 */
    uint64_t slaveConfChangeTime = 0;
    int slaveMasterLinkStatus = 0;
};


#endif //FLYSENTINEL_FLYINSTANCE_H
