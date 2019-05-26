//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYINSTANCE_H
#define FLYSENTINEL_FLYINSTANCE_H

#include "../coordinator/interface/AbstractFlyDBInstance.h"

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
    AbstractFlyDBInstance *getMaster() const;
    void setMaster(AbstractFlyDBInstance *master);

private:
    int flags;
    std::string name;
    SentinelAddr *addr = NULL;
    AbstractFlyDBInstance *master = NULL; /* Master instance if it's slave. */
};


#endif //FLYSENTINEL_FLYINSTANCE_H
