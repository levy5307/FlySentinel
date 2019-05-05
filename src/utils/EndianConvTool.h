//
// Created by 赵立伟 on 2018/10/4.
//

#ifndef FLYSENTINEL_ENDIANCONVTOOL_H
#define FLYSENTINEL_ENDIANCONVTOOL_H

#include <cstdint>
#include "../def.h"

class EndianConvTool {
public:
    static EndianConvTool* getInstance() {
        static EndianConvTool* instance = NULL;
        if (NULL == instance) {
            instance = new EndianConvTool();
        }
        return instance;
    }

    void memrev16ifbe(void* p) {
        if (BYTE_ORDER == BIG_ENDIAN) {
            memrev16(p);
        }
        return;
    }

    void memrev32ifbe(void* p) {
        if (BYTE_ORDER == BIG_ENDIAN) {
            memrev32(p);
        }
        return;
    }

    void memrev64ifbe(void* p) {
        if (BYTE_ORDER == BIG_ENDIAN) {
            memrev64(p);
        }
        return;
    }

    uint16_t intrev16ifbe(uint16_t v) {
        memrev16ifbe(&v);
        return v;
    }

    uint32_t intrev32ifbe(uint32_t v) {
        memrev32ifbe(&v);
        return v;
    }

    uint64_t intrev64ifbe(uint64_t v) {
        memrev64ifbe(&v);
        return v;
    }

private:
    EndianConvTool() {
    }

    void memrev16(void *p) {
        unsigned char *x = (unsigned char*)p, t;

        t = x[0];
        x[0] = x[1];
        x[1] = t;
    }

    void memrev32(void *p) {
        unsigned char *x = (unsigned char*)p, t;

        t = x[0];
        x[0] = x[3];
        x[3] = t;
        t = x[1];
        x[1] = x[2];
        x[2] = t;
    }

    void memrev64(void *p) {
        unsigned char *x = (unsigned char*)p, t;

        t = x[0];
        x[0] = x[7];
        x[7] = t;
        t = x[1];
        x[1] = x[6];
        x[6] = t;
        t = x[2];
        x[2] = x[5];
        x[5] = t;
        t = x[3];
        x[3] = x[4];
        x[4] = t;
    }
};


#endif //FLYSENTINEL_ENDIANCONVTOOL_H
