//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYINSTANCE_H
#define FLYSENTINEL_FLYINSTANCE_H

#include "../coordinator/interface/AbstractFlyDBInstance.h"

void sentinelDiscardReplyCallback(redisAsyncContext *context, void *reply, void *privdata);

class FlyDBInstance : public AbstractFlyDBInstance {
public:
    FlyDBInstance();
    ~FlyDBInstance();
    int getFlags() const;
    void setFlags(int flags);
    const std::string &getName() const;
    void setName(const std::string &name);
    SentinelAddr *getAddr() const;
    void setAddr(SentinelAddr *addr);
    std::shared_ptr<AbstractFlyDBInstance> getMaster() const;
    bool haveMaster() const;
    void setMaster(std::shared_ptr<AbstractFlyDBInstance> master);
    uint32_t getQuorum() const;
    void setQuorum(uint32_t quorum);
    char *getNotificationScript() const;
    void setNotificationScript(char *notificationScript);
    char *getClientReconfigScript() const;
    bool isClientReconfigScriptNULL() const;
    const std::shared_ptr<AbstractInstanceLink> &getLink() const;
    void setLink(const std::shared_ptr<AbstractInstanceLink> &link);
    void releaseLink();

private:
    int flags;
    std::string name;
    SentinelAddr *addr = NULL;
    std::shared_ptr<AbstractFlyDBInstance> master = NULL;
    uint32_t quorum;                                /** 对于判定flydb失败，需要的sentinel票数 */
    char *notificationScript = NULL;
    char *clientReconfigScript = NULL;
    std::shared_ptr<AbstractInstanceLink> link;     /** 与instance的连接，sentinel之间共享 */
};


#endif //FLYSENTINEL_FLYINSTANCE_H
