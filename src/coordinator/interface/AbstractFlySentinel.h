//
// Created by levy on 2019/5/5.
//

#ifndef ABSTRACTFLYSENTINEL_FLYSENTINEL_H
#define ABSTRACTFLYSENTINEL_FLYSENTINEL_H

#include "AbstractFlyInstance.h"

class AbstractFlySentinel : public AbstractFlyServer {
public:
    AbstractFlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache) : AbstractFlyServer(coordinator, configCache) {
    }
    virtual void sendEvent(int level, char *type, std::shared_ptr<AbstractFlyInstance> flyInstance, const char *fmt, ...) = 0;
    virtual void generateInitMonitorEvents() = 0;
    virtual int tryConnectionSharing(std::shared_ptr<AbstractFlyInstance> flyInstance) = 0;
    virtual void flushConfig() = 0;
    /** 将master的downAfterPeriod设置给与该master相连的所有sentinels和slaves */
    virtual void propagateDownAfterPeriod(std::shared_ptr<AbstractFlyInstance> master) = 0;
    virtual void refreshInstanceInfo(AbstractFlyInstance* flyInstance, const std::string &info) = 0;
};

#endif //ABSTRACTFLYSENTINEL_FLYSENTINEL_H
