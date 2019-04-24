//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjSkipListFactory.h"
#include "FlyObjSkipList.h"

FlyObjSkipListFactory::~FlyObjSkipListFactory() {

}

std::shared_ptr<FlyObj> FlyObjSkipListFactory::getObject() {
    return std::shared_ptr<FlyObj>(new FlyObjSkipList(FLY_TYPE_SKIPLIST));
}

std::shared_ptr<FlyObj> FlyObjSkipListFactory::getObject(void *ptr) {
    return std::shared_ptr<FlyObj>(new FlyObjSkipList(ptr, FLY_TYPE_SKIPLIST));
}
