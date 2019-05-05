//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYSENTINEL_FLYOBJHASHTABLE_H
#define FLYSENTINEL_FLYOBJHASHTABLE_H

#include "../FlyObj.h"

class FlyObjHashTable : public FlyObj {
public:
    FlyObjHashTable(FlyObjType type);
    FlyObjHashTable(void *ptr, FlyObjType type);
    ~FlyObjHashTable();
};


#endif //FLYSENTINEL_FLYOBJHASHTABLE_H
