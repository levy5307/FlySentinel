//
// Created by 赵立伟 on 2018/12/8.
//

#ifndef FLYSENTINEL_COORDINATOR_H
#define FLYSENTINEL_COORDINATOR_H

#include "interface/AbstractCoordinator.h"

class Coordinator : public AbstractCoordinator {
public:
    Coordinator();
    ~Coordinator();

    AbstractNetHandler *getNetHandler() const;
    AbstractFlyClientFactory *getFlyClientFactory() const;
    AbstractEventLoop *getEventLoop() const;

    /** fly object factory **/
    AbstractFlyObjFactory *getFlyObjHashTableFactory() const;
    AbstractFlyObjFactory *getFlyObjLinkedListFactory() const;
    AbstractFlyObjFactory *getFlyObjSkipListFactory() const;
    AbstractFlyObjFactory *getFlyObjIntSetFactory() const;
    AbstractFlyObjFactory *getFlyObjStringFactory() const;

    /** LogHandler */
    AbstractLogHandler *getLogHandler() const;

    /** bio */
    AbstractBIOHandler *getBioHandler() const;
    
private:
    AbstractNetHandler *netHandler;
    AbstractEventLoop *eventLoop;
    AbstractFlyClientFactory *flyClientFactory;
    AbstractFlyObjFactory *flyObjHashTableFactory;
    AbstractFlyObjFactory *flyObjLinkedListFactory;
    AbstractFlyObjFactory *flyObjSkipListFactory;
    AbstractFlyObjFactory *flyObjIntSetFactory;
    AbstractFlyObjFactory *flyObjStringFactory;
    ConfigCache *configCache;
    BaseConfigReader *configReader;


    /**
     * logHandler
     */
    AbstractLogHandler *logHandler = NULL;

    /**
     * bio
     **/
    AbstractBIOHandler *bioHandler = NULL;
};

#endif //FLYSENTINEL_COORDINATOR_H
