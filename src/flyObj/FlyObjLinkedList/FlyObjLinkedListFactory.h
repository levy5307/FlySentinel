//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJLINKEDLISTFACTORY_H
#define FLYDB_FLYOBJLINKEDLISTFACTORY_H

#include "../../coordinator/interface/AbstractFlyObjFactory.h"

class FlyObjLinkedListFactory : public AbstractFlyObjFactory {
public:
    ~FlyObjLinkedListFactory();
    std::shared_ptr<FlyObj> getObject();
    std::shared_ptr<FlyObj> getObject(void *ptr);
};


#endif //FLYDB_FLYOBJLINKEDLISTFACTORY_H
