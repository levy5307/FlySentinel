//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYSENTINEL_ABSTRACTCOORDINATOR_H
#define FLYSENTINEL_ABSTRACTCOORDINATOR_H

#include "AbstractEventLoop.h"
#include "AbstractNetHandler.h"
#include "AbstractFlyClientFactory.h"
#include "AbstractBIOHandler.h"
#include "AbstractLogHandler.h"
#include "AbstractFlyObjFactory.h"
#include "AbstractSharedObjects.h"
#include "AbstractPubSubHandler.h"
#include "AbstractFlyServer.h"

class AbstractCoordinator {
public:

    virtual AbstractNetHandler *getNetHandler() const = 0;

    virtual AbstractFlyClientFactory *getFlyClientFactory() const = 0;

    virtual AbstractEventLoop *getEventLoop() const = 0;

    /** fly object factory **/
    virtual AbstractFlyObjFactory *getFlyObjStringFactory() const = 0;

    virtual AbstractFlyServer* getFlyServer() const = 0;

    /** LogHandler */
    virtual AbstractLogHandler *getLogHandler() const = 0;

    /** bio */
    virtual AbstractBIOHandler *getBioHandler() const = 0;

    /** shared objects */
    virtual AbstractSharedObjects *getSharedObjects() const = 0;

    /** pub/sub */
    virtual AbstractPubSubHandler *getPubSubHandler() const = 0;
};

#endif //FLYSENTINEL_ABSTRACTCOORDINATOR_H
