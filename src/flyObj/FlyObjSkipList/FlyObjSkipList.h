//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYSENTINEL_FLYOBJSKIPLIST_H
#define FLYSENTINEL_FLYOBJSKIPLIST_H

#include "../FlyObjDef.h"
#include "../FlyObj.h"

class FlyObjSkipList : public FlyObj {
public:
    FlyObjSkipList(FlyObjType type);
    FlyObjSkipList(void *ptr, FlyObjType type);
    ~FlyObjSkipList();
};


#endif //FLYSENTINEL_FLYOBJSKIPLIST_H
