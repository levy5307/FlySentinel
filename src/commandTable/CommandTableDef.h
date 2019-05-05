//
// Created by 赵立伟 on 2018/12/23.
//

#ifndef FLYSENTINEL_COMMANDTABLEDEF_H
#define FLYSENTINEL_COMMANDTABLEDEF_H

enum CommandFlag {
    CMD_WRITE = 1 << 0,                 /** "w" flag */
    CMD_READONLY = 1 << 1,              /** "r" flag */
    CMD_DENYOOM = 1 << 2,               /** "m" flag */
    CMD_MODULE = 1 << 3,                /** Command exported by module. */
    CMD_ADMIN = 1 << 4,                 /** "a" flag */
    CMD_PUBSUB = 1 << 5,                /** "p" flag */
    CMD_NOSCRIPT = 1 << 6,              /** "s" flag */
    CMD_RANDOM = 1 << 7,                /** "R" flag */
    CMD_SORT_FOR_SCRIPT = 1 << 8,       /** "S" flag */
    CMD_LOADING = 1 << 9,               /** "l" flag */
    CMD_STALE = 1 << 10,                /** "t" flag */
    CMD_SKIP_MONITOR = 1 << 11,         /** "M" flag */
    CMD_ASKING = 1 << 12,               /** "k" flag */
    CMD_FAST = 1 << 13,                 /** "F" flag */
    CMD_MODULE_GETKEYS = 1 << 14,       /** Use the modules getkeys interface. */
    CMD_MODULE_NO_CLUSTER = 1 << 15     /** Deny on Cluster. */
};

#endif //FLYSENTINEL_COMMANDTABLEDEF_H
