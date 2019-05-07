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
#include "../flySentinel/FlySentinel.h"

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

    /** flyServer */
    this->flyServer = new FlySentinel(this);

    /** event loop **/
    this->eventLoop = new EventLoop(this, flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    // 时间循环处理器
    this->eventLoop->createTimeEvent(1, serverCron, NULL, NULL);


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

AbstractFlyServer* Coordinator::getFlyServer() const {
    return this->flyServer;
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
