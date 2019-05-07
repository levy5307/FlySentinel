//
// Created by 赵立伟 on 2018/12/8.
//

#include "Coordinator.h"
#include "../net/NetHandler.h"
#include "../config/TextConfigReader.h"
#include "../flyClient/FlyClientFactory.h"
#include "../bio/BIOHandler.h"
#include "../def.h"
#include "../event/EventLoop.h"

Coordinator::Coordinator() {
    /** 加载config **/
    std::string configfile = "../conf/sentinel.conf";                    /** 配置文件名字 */
    this->configReader = new TextConfigReader(configfile);
    this->configCache = configReader->loadConfig();

    /** client factory **/
    this->flyClientFactory = new FlyClientFactory();

    /** net handler **/
    this->netHandler = NetHandler::getInstance();

    /** logger初始化 */
    FileLogFactory::init(configCache->getLogfile(),
                         configCache->getSyslogEnabled(),
                         configCache->getVerbosity());
    if (configCache->getSyslogEnabled()) {          /** syslog */
        openlog(configCache->getSyslogIdent(),
                LOG_PID | LOG_NDELAY | LOG_NOWAIT,
                configCache->getSyslogFacility());
    }

    /** event loop **/
    //this->eventLoop = new EventLoop(this, flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    /** background io*/
    this->bioHandler = new BIOHandler();

    /** log handler */
    this->logHandler = logFactory->getLogger();
}

Coordinator::~Coordinator() {
    delete this->eventLoop;
    delete this->netHandler;
    delete this->flyClientFactory;
    delete this->bioHandler;
    delete this->logHandler;
}

AbstractNetHandler *Coordinator::getNetHandler() const {
    return this->netHandler;
}

AbstractEventLoop *Coordinator::getEventLoop() const {
    return this->eventLoop;
}

AbstractFlyClientFactory *Coordinator::getFlyClientFactory() const {
    return this->flyClientFactory;
}

AbstractFlyObjFactory *Coordinator::getFlyObjStringFactory() const {
    return flyObjStringFactory;
}

AbstractLogHandler *Coordinator::getLogHandler() const {
    return this->logHandler;
}

AbstractBIOHandler *Coordinator::getBioHandler() const {
    return this->bioHandler;
}
