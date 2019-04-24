//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjStringFactory.h"
#include "FlyObjString.h"

FlyObjStringFactory::~FlyObjStringFactory() {

}

std::shared_ptr<FlyObj> FlyObjStringFactory::getObject(void *ptr) {
    return std::shared_ptr<FlyObj>(new FlyObjString(ptr, FLY_TYPE_STRING));
}

std::shared_ptr<FlyObj> FlyObjStringFactory::getObject() {
    return std::shared_ptr<FlyObj>(new FlyObjString(FLY_TYPE_STRING));
}
