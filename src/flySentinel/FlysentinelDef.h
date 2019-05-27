//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_FLYSENTINELDEF_H
#define FLYSENTINEL_FLYSENTINELDEF_H

const int FLYDB_SENTINEL_PORT = 26379;

enum FlySentinelInstance {
    FSI_MASTER = 1 << 0,
    FSI_SLAVE = 1 << 1,
    FSI_SENTINEL = 1 << 2,
    FSI_S_DOWN = 1 << 3,        /** subject down */
    FSI_O_DOWN = 1 << 4,        /** object down */
    FSI_MASTER_DOWN = 1 << 5    /** 如果设置了该标志位，说明其master已经down */
};

#endif //FLYSENTINEL_FLYSENTINELDEF_H
