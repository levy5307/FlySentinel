//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYSENTINEL_COORDINATOR_H
#define FLYSENTINEL_COORDINATOR_H

#include "interface/AbstractCoordinator.h"
#include "../config/ConfigCache.h"
#include "../config/base/BaseConfigReader.h"
#include "interface/AbstractPubSubHandler.h"

class Coordinator : public AbstractCoordinator {
public:
    Coordinator();
    ~Coordinator();

    AbstractNetHandler *getNetHandler() const;
    AbstractFlyClientFactory *getFlyClientFactory() const;
    AbstractEventLoop *getEventLoop() const;

    /** fly object factory **/
    AbstractFlyObjFactory *getFlyObjStringFactory() const;

    /** fly server */
    AbstractFlyServer* getFlyServer() const;

    /** LogHandler */
    AbstractLogHandler *getLogHandler() const;

    /** bio */
    AbstractBIOHandler *getBioHandler() const;

    /** shared objects */
    AbstractSharedObjects *getSharedObjects() const;

    /** pub/sub */
    AbstractPubSubHandler *getPubSubHandler() const;

private:
    void createEvent(const std::vector<int> &ipfd);

    AbstractNetHandler *netHandler;
    AbstractEventLoop *eventLoop;
    AbstractFlyClientFactory *flyClientFactory;
    ConfigCache *configCache;
    BaseConfigReader *configReader;
    AbstractFlyObjFactory *flyObjStringFactory;

    /** shared objects */
    AbstractSharedObjects *sharedObjects;

    /**
     * logHandler
     */
    AbstractLogHandler *logHandler = nullptr;

    /**
     * bio
     **/
    AbstractBIOHandler *bioHandler = nullptr;

    /**
     * fly server
     **/
    AbstractFlyServer *flyServer;

    /** pub/sub handler */
    AbstractPubSubHandler *pubSubHandler;
};

#endif //FLYSENTINEL_COORDINATOR_H
