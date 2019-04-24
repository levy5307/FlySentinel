//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJSTRINGFACTORY_H
#define FLYDB_FLYOBJSTRINGFACTORY_H

#include "../interface/AbstractFlyObjFactory.h"

class FlyObjStringFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjStringFactory();
    std::shared_ptr<FlyObj> getObject(void *ptr);
    std::shared_ptr<FlyObj> getObject();
};


#endif //FLYDB_FLYOBJSTRINGFACTORY_H
