//
// Created by 赵立伟 on 2018/12/9.
//

#ifndef FLYSENTINEL_FLYOBJSTRING_H
#define FLYSENTINEL_FLYOBJSTRING_H

#include "../FlyObjDef.h"
#include "../FlyObj.h"

class FlyObjString : public FlyObj {
public:
    FlyObjString(FlyObjType type);
    FlyObjString(void *ptr, FlyObjType type);
    ~FlyObjString();
};


#endif //FLYDB_FLYOBJSTRING_H
