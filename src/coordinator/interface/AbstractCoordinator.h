//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYDB_ABSTRACTCOORDINATOR_H
#define FLYDB_ABSTRACTCOORDINATOR_H

#include "AbstractEventLoop.h"
#include "AbstractNetHandler.h"
#include "../../config/base/BaseConfigReader.h"
#include "AbstractFlyClientFactory.h"
#include "AbstractFlyObjFactory.h"
#include "../../bio/interface/AbstractBIOHandler.h"

class AbstractFlyServer;
class AbstractNetHandler;
class AbstractFlyClientFactory;

class AbstractCoordinator {
public:

    virtual AbstractNetHandler *getNetHandler() const = 0;

    virtual AbstractFlyClientFactory *getFlyClientFactory() const = 0;

    virtual AbstractEventLoop *getEventLoop() const = 0;

    /** fly object factory **/
    virtual AbstractFlyObjFactory *getFlyObjHashTableFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjLinkedListFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjSkipListFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjIntSetFactory() const = 0;
    virtual AbstractFlyObjFactory *getFlyObjStringFactory() const = 0;

    /** LogHandler */
    virtual AbstractLogHandler *getLogHandler() const = 0;

    /** bio */
    virtual AbstractBIOHandler *getBioHandler() const = 0;
};

#endif //FLYDB_ABSTRACTCOORDINATOR_H
