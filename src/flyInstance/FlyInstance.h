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

class FlyInstance : public AbstractFlyInstance {
public:
    FlyInstance(const std::string &name, int flags, const std::string &hostname,
                int port, int quorum, std::shared_ptr<AbstractFlyInstance> master);
    ~FlyInstance();
    int getFlags() const;
    void setFlags(int flags);
    const std::string &getName() const;
    void setName(const std::string &name);
    SentinelAddr *getAddr() const;
    void setAddr(SentinelAddr *addr);
    void dupAddr(SentinelAddr *addr);
    std::shared_ptr<AbstractFlyInstance> getMaster() const;
    bool haveMaster() const;
    void setMaster(std::shared_ptr<AbstractFlyInstance> master);
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
    const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &getSentinels() const;
    const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &getSlaves() const;
    std::shared_ptr<AbstractFlyInstance> lookupSlave(char *ip, int port);
    int removeMatchingSentinel(char *runid);
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
    bool sendPing();
    int sendHello();
    const std::shared_ptr<AbstractFlyInstance> &getPromotedSlave() const;
    void setPromotedSlave(const std::shared_ptr<AbstractFlyInstance> &promotedSlave);
    bool hasPromotedSlave() const;
    FailoverState getFailoverState() const;
    void setFailoverState(FailoverState failoverState);
    void clearInfo();
    void setInfo(const std::string &info);
    const std::string& getInfo() const;
    uint64_t getConfigEpoch() const;
    void setConfigEpoch(uint64_t configEpoch);

private:
    FlyInstance(){};
    int flags;
    std::string name;
    std::string runid;
    uint64_t configEpoch;           /** 配置epoch */
    SentinelAddr *addr = NULL;
    std::shared_ptr<AbstractFlyInstance> master = NULL;
    uint32_t quorum;                                /** 对于判定flydb失败，需要的sentinel票数 */
    char *notificationScript = NULL;
    char *clientReconfigScript = NULL;
    std::shared_ptr<AbstractInstanceLink> link;     /** 与instance的连接，sentinel之间共享 */
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>> sentinels;    /** key-ip:port. Other sentinels monitoring the same master. */
    std::map<std::string, std::shared_ptr<AbstractFlyInstance>> slaves;       /** key-ip:port. Slaves for this master instance. */
    uint64_t sDownSinceTime = 0;                        /** Subjectively down since time. */
    uint64_t oDownSinceTime = 0;                        /** Objectively down since time. */
    uint64_t downAfterPeriod = 0;                       /** down after this period */
    std::shared_ptr<AbstractFlyInstance> promotedSlave = NULL;
    FailoverState failoverState = SENTINEL_FAILOVER_STATE_NONE;
    int roleReported;
    uint64_t roleReportedTime = 0;
    uint64_t infoRefresh = 0;
    std::string info;                                   /** 缓存的输出info */
};


#endif //FLYSENTINEL_FLYINSTANCE_H
