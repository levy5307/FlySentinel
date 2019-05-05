//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYSENTINEL_FLYOBJSKIPLISTFACTORY_H
#define FLYSENTINEL_FLYOBJSKIPLISTFACTORY_H

#include "../../coordinator/interface/AbstractFlyObjFactory.h"

class FlyObjSkipListFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjSkipListFactory();
    std::shared_ptr<FlyObj> getObject();
    std::shared_ptr<FlyObj> getObject(void *ptr);
};


#endif //FLYSENTINEL_FLYOBJSKIPLISTFACTORY_H
