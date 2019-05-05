//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYSENTINEL_FLYOBJLINKEDLIST_H
#define FLYSENTINEL_FLYOBJLINKEDLIST_H

#include "../FlyObjDef.h"
#include "../FlyObj.h"

class FlyObjLinkedList : public FlyObj {
public:
    FlyObjLinkedList(FlyObjType type);
    FlyObjLinkedList(void *ptr, FlyObjType type);
    ~FlyObjLinkedList();
};


#endif //FLYSENTINEL_FLYOBJLINKEDLIST_H
