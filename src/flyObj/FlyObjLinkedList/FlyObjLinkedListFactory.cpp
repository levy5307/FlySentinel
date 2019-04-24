//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjLinkedListFactory.h"
#include "FlyObjLinkedList.h"

FlyObjLinkedListFactory::~FlyObjLinkedListFactory() {

}

std::shared_ptr<FlyObj> FlyObjLinkedListFactory::getObject() {
    return std::shared_ptr<FlyObj>(new FlyObjLinkedList(FLY_TYPE_LIST));
}

std::shared_ptr<FlyObj> FlyObjLinkedListFactory::getObject(void *ptr) {
    return std::shared_ptr<FlyObj>(new FlyObjLinkedList(ptr, FLY_TYPE_LIST));
}
