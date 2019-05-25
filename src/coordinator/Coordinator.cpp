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
#include "../shared/SharedObjects.h"
#include "../flyObj/FlyObjString/FlyObjStringFactory.h"
#include "../pubsub/PubSubHandler.h"

Coordinator::Coordinator() {
    /** 加载config **/
    std::string configfile = "../conf/sentinel.conf";                    /** 配置文件名字 */
    this->configReader = new TextConfigReader(configfile);
    this->configCache = configReader->loadConfig();

    /** client factory **/
    this->flyClientFactory = new FlyClientFactory();

    /** fly obj factory **/
    this->flyObjStringFactory = new FlyObjStringFactory();

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
    this->flyServer = new FlySentinel(this, this->configCache);

    /** event loop **/
    this->eventLoop = new EventLoop(this, flyServer->getMaxClients() + CONFIG_FDSET_INCR);

    /** 创建事件 */
    this->createEvent(flyServer->getIpfd());

    /** background io*/
    this->bioHandler = new BIOHandler();

    /** shared objects */
    this->sharedObjects = new SharedObjects(this);

    /** log handler */
    this->logHandler = logFactory->getLogger();
    
    /** pub/sub handler */
    this->pubSubHandler = new PubSubHandler(this);
}

Coordinator::~Coordinator() {
    delete this->eventLoop;
    delete this->netHandler;
    delete this->flyClientFactory;
    delete this->bioHandler;
    delete this->logHandler;
    delete this->pubSubHandler;
}

void Coordinator::createEvent(const std::vector<int> &ipfd) {
    // 时间循环处理器
    this->eventLoop->createTimeEvent(1, serverCron, NULL, NULL);
    // 创建定时任务，用于创建客户端连接
    for (auto fd : ipfd) {
        if (-1 == this->eventLoop->createFileEvent(
                fd, ES_READABLE, acceptTcpHandler, nullptr)) {
            exit(1);
        }
    }
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

AbstractSharedObjects *Coordinator::getSharedObjects() const {
    return sharedObjects;
}

AbstractPubSubHandler *Coordinator::getPubSubHandler() const {
    return this->pubSubHandler;
}
