//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_CLIENTDEF_H
#define FLYDB_CLIENTDEF_H

/** 当前服务器是一个slave */
const int CLIENT_SLAVE = 1 << 0;
/** 当前服务器是一个master */
const int CLIENT_MASTER = 1 << 1;
/** 回复后关闭(即处理完该请求后关闭) */
const int CLIENT_CLOSE_AFTER_REPLY = 1 << 6;
/** client中有需要发送的数据，但是write handler还没有install */
const int CLIENT_PENDING_WRITE = 1 << 21;
/** 异步地关闭该client */
const int CLIENT_CLOSE_ASAP = 1 << 10;
/** 对于master是不接收reply的，但是如果设置了该flag，则即使是master也要接收reply **/
const int CLIENT_MASTER_FORCE_REPLY = 1 << 13;
/** Instance don't understand PSYNC */
const int CLIENT_PRE_PSYNC = 1 << 16;

const int FLY_REPLY_CHUNK_BYTES = 16 * 1024;

#endif //FLYDB_CLIENTDEF_H
