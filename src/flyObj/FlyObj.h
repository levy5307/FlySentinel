//
// Created by levy on 2019/5/6.
//

#ifndef FLYSENTINEL_FLYOBJ_H
#define FLYSENTINEL_FLYOBJ_H

#include <memory>
#include <cstdint>
#include "FlyObjDef.h"

class FlyObj {
public:
    FlyObj(FlyObjType type);
    FlyObj(void* ptr, FlyObjType type);
    ~FlyObj();
    FlyObjType getType() const;
    void setType(FlyObjType type);
    uint64_t getLru() const;
    void setLru(uint64_t lru);
    void *getPtr() const;

protected:
    FlyObjType type;
    uint64_t lru;
    void* ptr;
};



#endif //FLYSENTINEL_FLYOBJ_H
