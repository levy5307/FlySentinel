//
// Created by levy on 2019/5/6.
//

#include "FlyObj.h"
#include <string>
#include <list>
#include <mach/mach.h>
#include "FlyObj.h"
#include "FlyObjDef.h"

FlyObj::FlyObj(FlyObjType type) {
    this->type = type;
}

FlyObj::FlyObj(void* ptr, FlyObjType type) {
    this->ptr = ptr;
    this->type = type;
}

FlyObj::~FlyObj() {

}

FlyObjType FlyObj::getType() const {
    return this->type;
}

void FlyObj::setType(FlyObjType type) {
    this->type = type;
}

uint64_t FlyObj::getLru() const {
    return this->lru;
}

void FlyObj::setLru(uint64_t lru) {
    this->lru = lru;
}

void *FlyObj::getPtr() const {
    return this->ptr;
}
