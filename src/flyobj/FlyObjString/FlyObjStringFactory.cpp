//
// Created by levy on 2019/5/7.
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
