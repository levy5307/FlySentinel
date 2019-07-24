//
// Created by levy on 2019/5/15.
//

#ifndef FLYSENTINEL_FLYSENTINELDEF_H
#define FLYSENTINEL_FLYSENTINELDEF_H

#include "../def.h"

const int FLYDB_SENTINEL_PORT = 26379;
const int SENTINEL_SCRIPT_MAX_ARGS = 16;
const int SENTINEL_SCRIPT_MAX_QUEUE = 256;
const int SENTINEL_SCRIPT_MAX_RUNNING = 16;
const int SENTINEL_SCRIPT_MAX_RETRY = 10;
const int SENTINEL_LEADER = 1 << 17;
const int SENTINEL_OBSERVER = 1 << 18;
const int SENTINEL_GENERATE_EVENT = 1 << 16;

enum FlySentinelInstance {
    FSI_MASTER = 1 << 0,
    FSI_SLAVE = 1 << 1,
    FSI_SENTINEL = 1 << 2,
    FSI_S_DOWN = 1 << 3,                    /** subject down */
    FSI_O_DOWN = 1 << 4,                    /** object down */
    FSI_MASTER_DOWN = 1 << 5,               /** 如果设置了该标志位，说明其master已经down */
    FSI_FAILOVER_IN_PROGRESS = 1 << 6,      /** Failover is in progress for */
    FSI_PROMOTED = 1 << 7,                  /** Slave selected for promotion. */
    FSI_RECONF_SENT = 1 << 8,               /** SLAVEOF <newmaster> sent. */
    FSI_RECONF_INPROG = 1 << 9,             /** Slave synchronization in progress. */
    FSI_RECONF_DONE = 1 << 10,              /** Slave synchronized with new master. */
    FSI_FORCE_FAILOVER = 1 << 11,           /** Force failover with master up. */
    FSI_SCRIPT_KILL_SENT = 1 << 12          /** SCRIPT KILL already sent on -BUSY */
};

enum FlySentinelQuorumState {
    FSQS_OK = 0,
    FSQS_NOQUORUM = 1,
    FSQS_NOVOTER = 2
};

const uint8_t HELLO_SIZE = 8;
const int SENTINEL_MASTER_LINK_STATUS_UP = 0;
const int SENTINEL_MASTER_LINK_STATUS_DOWN = 1;
const int SENTINEL_TILT_TRIGGER = 2000;
const int SENTINEL_TILT_PERIOD = SENTINEL_PING_PERIOD * 30;

#endif //FLYSENTINEL_FLYSENTINELDEF_H
