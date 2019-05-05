//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJHASHTABLEFACTORY_H
#define FLYDB_FLYOBJHASHTABLEFACTORY_H

#include "../../coordinator/interface/AbstractFlyObjFactory.h"

class FlyObjHashTableFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjHashTableFactory();
    std::shared_ptr<FlyObj> getObject();
    std::shared_ptr<FlyObj> getObject(void *ptr);

};


#endif //FLYDB_FLYOBJHASHTABLEFACTORY_H
