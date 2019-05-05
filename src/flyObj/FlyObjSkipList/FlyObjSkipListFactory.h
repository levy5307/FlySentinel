//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJSKIPLISTFACTORY_H
#define FLYDB_FLYOBJSKIPLISTFACTORY_H

#include "../../coordinator/interface/AbstractFlyObjFactory.h"

class FlyObjSkipListFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjSkipListFactory();
    std::shared_ptr<FlyObj> getObject();
    std::shared_ptr<FlyObj> getObject(void *ptr);
};


#endif //FLYDB_FLYOBJSKIPLISTFACTORY_H
