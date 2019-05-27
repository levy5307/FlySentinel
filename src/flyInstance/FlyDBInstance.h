//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYINSTANCE_H
#define FLYSENTINEL_FLYINSTANCE_H

#include "../coordinator/interface/AbstractFlyDBInstance.h"
#include "../flySentinel/SentinelAddr.h"

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
    AbstractFlyDBInstance* getMaster() const;
    bool haveMaster() const;
    void setMaster(AbstractFlyDBInstance *master);
    uint32_t getQuorum() const;
    void setQuorum(uint32_t quorum);

private:
    int flags;
    std::string name;
    SentinelAddr *addr = NULL;
    AbstractFlyDBInstance *master = NULL;
    uint32_t quorum;            /** 对于判定flydb失败，需要的sentinel票数 */
};


#endif //FLYSENTINEL_FLYINSTANCE_H
