//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjHashTableFactory.h"
#include "FlyObjHashTable.h"

FlyObjHashTableFactory::~FlyObjHashTableFactory() {

}

std::shared_ptr<FlyObj> FlyObjHashTableFactory::getObject() {
    return std::shared_ptr<FlyObj>(new FlyObjHashTable(FLY_TYPE_HASH));
}

std::shared_ptr<FlyObj> FlyObjHashTableFactory::getObject(void *ptr) {
    return std::shared_ptr<FlyObj>(new FlyObjHashTable(ptr, FLY_TYPE_HASH));
}
