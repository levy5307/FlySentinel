//
// Created by levy on 2019/5/8.
//

#ifndef FLYSENTINEL_ABSTRACTFLYINSTANCE_H
#define FLYSENTINEL_ABSTRACTFLYINSTANCE_H

#include <string>
#include "../../flySentinel/SentinelAddr.h"
#include "AbstractInstanceLink.h"

class AbstractFlyInstance {
public:
    virtual int getFlags() const = 0;
    virtual void setFlags(int flags) = 0;
    virtual const std::string &getName() const = 0;
    virtual void setName(const std::string &name) = 0;
    virtual SentinelAddr *getAddr() const = 0;
    virtual void setAddr(SentinelAddr *addr) = 0;
    virtual void dupAddr(SentinelAddr *addr) = 0;
    virtual std::shared_ptr<AbstractFlyInstance> getMaster() const = 0;
    virtual bool haveMaster() const = 0;
    virtual void setMaster(std::shared_ptr<AbstractFlyInstance> master) = 0;
    virtual uint32_t getQuorum() const = 0;
    virtual void setQuorum(uint32_t quorum) = 0;
    virtual char *getNotificationScript() const = 0;
    virtual void setNotificationScript(char *notificationScript) = 0;
    virtual char *getClientReconfigScript() const = 0;
    virtual bool isClientReconfigScriptNULL() const = 0;
    virtual const std::shared_ptr<AbstractInstanceLink> &getLink() const = 0;
    virtual void setLink(const std::shared_ptr<AbstractInstanceLink> &link) = 0;
    virtual void releaseLink() = 0;
    virtual const std::string &getRunid() const = 0;
    virtual void setRunid(const std::string &runid) = 0;
    virtual const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &getSentinels() const = 0;
    virtual const std::map<std::string, std::shared_ptr<AbstractFlyInstance>> &getSlaves() const = 0;
    virtual std::shared_ptr<AbstractFlyInstance> lookupSlave(char *ip, int port) = 0;
    virtual int removeMatchingSentinel(char *runid) = 0;
};

#endif //FLYSENTINEL_ABSTRACTFLYINSTANCE_H
