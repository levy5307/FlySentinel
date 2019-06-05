//
// Created by levy on 2019/6/3.
//

#ifndef FLYSENTINEL_FLYINSTANCEDEF_H
#define FLYSENTINEL_FLYINSTANCEDEF_H

const int SENTINEL_RESET_SENTINELS = 1 << 0;
const int SENTINEL_DEFAULT_DOWN_AFTER = 30 * 1000;

enum FailoverState {
    SENTINEL_FAILOVER_STATE_NONE = 0,                  /** No failover in progress. */
    SENTINEL_FAILOVER_STATE_WAIT_START = 1,            /** Wait for failover_start_time*/
    SENTINEL_FAILOVER_STATE_SELECT_SLAVE = 2,          /** Select slave to promote */
    SENTINEL_FAILOVER_STATE_SEND_SLAVEOF_NOONE = 3,    /** Slave -> Master */
    SENTINEL_FAILOVER_STATE_WAIT_PROMOTION = 4,        /** Wait slave to change role */
    SENTINEL_FAILOVER_STATE_RECONF_SLAVES = 5,         /** SLAVEOF newmaster */
    SENTINEL_FAILOVER_STATE_UPDATE_CONFIG = 6          /** Monitor promoted slave. */
};
#endif //FLYSENTINEL_FLYINSTANCEDEF_H
