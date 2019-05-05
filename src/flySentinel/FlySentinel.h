//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYSENTINEL_H
#define FLYSENTINEL_FLYSENTINEL_H

#include "../coordinator/interface/AbstractFlySentinel.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class FlySentinel : public AbstractFlySentinel {
public:

private:
    AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_FLYSENTINEL_H
