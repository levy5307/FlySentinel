//
// Created by levy on 2019/5/7.
//

#include <string>
#include "FlyObjString.h"

FlyObjString::FlyObjString(FlyObjType type) : FlyObj(new std::string, type) {
}

FlyObjString::FlyObjString(void *ptr, FlyObjType type) : FlyObj(ptr, type) {

}

FlyObjString::~FlyObjString() {
    delete reinterpret_cast<std::string*>(this->ptr);
}
