//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYDB_FLYOBJSKIPLIST_H
#define FLYDB_FLYOBJSKIPLIST_H

#include "../FlyObjDef.h"
#include "../FlyObj.h"

class FlyObjSkipList : public FlyObj {
public:
    FlyObjSkipList(FlyObjType type);
    FlyObjSkipList(void *ptr, FlyObjType type);
    ~FlyObjSkipList();
};


#endif //FLYDB_FLYOBJSKIPLIST_H
