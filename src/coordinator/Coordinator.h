//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYSENTINEL_COORDINATOR_H
#define FLYSENTINEL_COORDINATOR_H

#include "interface/AbstractCoordinator.h"
#include "../config/ConfigCache.h"
#include "../config/base/BaseConfigReader.h"

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
    
private:
    AbstractNetHandler *netHandler;
    AbstractEventLoop *eventLoop;
    AbstractFlyClientFactory *flyClientFactory;
    ConfigCache *configCache;
    BaseConfigReader *configReader;
    AbstractFlyObjFactory *flyObjStringFactory;
    AbstractFlyServer *flyServer;

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

};

#endif //FLYSENTINEL_COORDINATOR_H
