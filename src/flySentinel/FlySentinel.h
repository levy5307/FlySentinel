//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYSENTINEL_H
#define FLYSENTINEL_FLYSENTINEL_H

#include <map>
#include <vector>
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../config/ConfigCache.h"
#include "../def.h"
#include "../coordinator/interface/AbstractFlyInstance.h"

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData);

class FlySentinel : public AbstractFlyServer {
public:
    FlySentinel(const AbstractCoordinator *coordinator, ConfigCache *configCache);
    ~FlySentinel();

private:
    char myid[CONFIG_RUN_ID_SIZE + 1];
    uint64_t currentEpoch = 0;
    std::map<std::string, std::shared_ptr<AbstractFlyInstance> > *masters;
    bool tilt = false;                 /** tilt mode */
    uint64_t tiltStartTime = 0;
    uint64_t previousTime = miscTool->mstime();
    char *announceIP = NULL;
    int announcePort = 0;
};


#endif //FLYSENTINEL_FLYSENTINEL_H
