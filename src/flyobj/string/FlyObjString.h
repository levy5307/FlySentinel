//
// Created by levy on 2019/5/7.
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


#endif //FLYSENTINEL_FLYOBJSTRING_H
