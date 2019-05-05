//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYSENTINEL_NETDEF_H
#define FLYSENTINEL_NETDEF_H

const int NET_NONE = 0;
const int NET_IP_ONLY = 1<<0;

enum NetConnectFlag {
    NET_CONNECT_NONE = 0,
    NET_CONNECT_NONBLOCK = 1,
    NET_CONNECT_BE_BINDING = 2          // Best effort binding
};

const int MAX_ACCEPTS_PER_CALL = 1000;
const int NET_IP_STR_LEN = 46;
const int NET_PEER_ID_LEN = NET_IP_STR_LEN + 32;            /** Must be enough for ip:port */
const int NET_ERR_LEN = 256;                                // 网络error信息buffer
const int NET_MAX_WRITES_PER_EVENT = 1024 * 64;

/**
 * 协议相关
 */
const int PROTO_IOBUF_LEN = 1024 * 16;                      // Generic I/O buffer size
const int PROTO_MAX_QUERYBUF_LEN = 1024 * 1024 * 1024;      // client query buf最大长度（1GB)
// request type
const int PROTO_REQ_INLINE = 1;
const int PROTO_REQ_MULTIBULK = 2;
const int PROTO_REQ_MULTIBULK_MAX_LEN = 1024 * 1024;
const int PROTO_REQ_BULK_MAX_LEN = 512 * 1024 * 1024;
const int PROTO_INLINE_MAX_SIZE = 1024 * 64;                // 每行的最大长度
const int PROTO_REPLY_CHUNK_BYTES = 16 * 1024;              // flyclient每个reply最大长度

#endif //FLYSENTINEL_NETDEF_H
