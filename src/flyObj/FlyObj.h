//
// Created by 赵立伟 on 2018/10/13.
//

#ifndef FLYDB_FLYOBJ_H
#define FLYDB_FLYOBJ_H

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

#endif //FLYDB_FLYOBJ_H
