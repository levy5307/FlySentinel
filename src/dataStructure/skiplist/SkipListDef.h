//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYSENTINEL_SKIPDEF_H
#define FLYSENTINEL_SKIPDEF_H

const uint8_t SKIP_LIST_MAX_LEVEL = 32;

struct SkipListRange {
    SkipListRange(double min, double max, bool minex, bool maxex)
            : min(min), max(max), minex(minex), maxex(maxex) {

    }

    double min, max;
    bool minex, maxex;
};

#endif //FLYSENTINEL_SKIPDEF_H
