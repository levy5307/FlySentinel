//
// Created by 赵立伟 on 2018/12/9.
//

#include "FlyObjHashTable.h"
#include "../../dataStructure/dict/Dict.h"
#include "../FlyObjDeleter.h"

FlyObjHashTable::FlyObjHashTable(FlyObjType type)
        : FlyObj(new Dict<std::string, std::string>(), type) {
}

FlyObjHashTable::FlyObjHashTable(void *ptr, FlyObjType type)
        : FlyObj(ptr, type) {
}

FlyObjHashTable::~FlyObjHashTable() {
    delete reinterpret_cast<Dict<std::string, std::string>*>(this->ptr);
}
