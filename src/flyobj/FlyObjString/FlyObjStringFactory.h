//
// Created by levy on 2019/5/7.
//

#ifndef FLYSENTINEL_FLYOBJSTRINGFACTORY_H
#define FLYSENTINEL_FLYOBJSTRINGFACTORY_H

#include "../../coordinator/interface/AbstractFlyObjFactory.h"

class FlyObjStringFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjStringFactory();
    std::shared_ptr<FlyObj> getObject(void *ptr);
    std::shared_ptr<FlyObj> getObject();
};



#endif //FLYSENTINEL_FLYOBJSTRINGFACTORY_H
