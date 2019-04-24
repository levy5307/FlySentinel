//
// Created by 赵立伟 on 2018/12/9.
//

#include <string>
#include <list>
#include "FlyObjLinkedList.h"
#include "../../dataStructure/skiplist/SkipList.h"
#include "../FlyObjDeleter.h"

FlyObjLinkedList::FlyObjLinkedList(FlyObjType type)
        : FlyObj(new std::list<std::string>, type) {
}

FlyObjLinkedList::FlyObjLinkedList(void *ptr, FlyObjType type)
        : FlyObj(ptr, type) {
}

FlyObjLinkedList::~FlyObjLinkedList() {
    delete reinterpret_cast<std::list<std::string> *>(this->ptr);
}
