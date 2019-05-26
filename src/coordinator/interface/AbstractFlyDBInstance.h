//
// Created by levy on 2019/5/8.
//

#ifndef FLYSENTINEL_ABSTRACTFLYINSTANCE_H
#define FLYSENTINEL_ABSTRACTFLYINSTANCE_H

#include <string>
#include "../../flySentinel/FlysentinelDef.h"

class AbstractFlyDBInstance {
public:
    virtual int getFlags() const = 0;
    virtual void setFlags(int flags) = 0;
    virtual const std::string &getName() const = 0;
    virtual void setName(const std::string &name) = 0;
    virtual SentinelAddr *getAddr() const = 0;
    virtual void setAddr(SentinelAddr *addr) = 0;
    virtual AbstractFlyDBInstance *getMaster() const = 0;
    virtual void setMaster(AbstractFlyDBInstance *master) = 0;

};

#endif //FLYSENTINEL_ABSTRACTFLYINSTANCE_H
