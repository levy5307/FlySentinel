//
// Created by levy on 2019/3/29.
//

#include "SocketFio.h"
#include "../net/NetDef.h"
#include <sys/socket.h>
#include <cerrno>

SocketFio::SocketFio() {
    this->pos = 0;
}

int SocketFio::tell() {
    return this->pos;
}

int SocketFio::flush() {
    int broken = 0;
    const char *p = this->buf.c_str();

    /** 循环写入sockets */
    int length = this->buf.length();
    while (length > 0) {
        /** 分批次写入，每次最多写1024，以便于并行的向所有的sockets循环发送 */
        int count = this->buf.length() > 1024 ? 1024 : this->buf.length();

        /** 将count大小的数据写入所有的sockets */
        for (int i = 0; i < this->fds.size(); i++) {
            /** 已经被标记为失败了, 则处理下一个 */
            if (0 != this->states[i]) {
                continue;
            }

            /** 写入该socket */
            int nwritten = 0;
            while (nwritten < count) {
                int res = send(this->fds[i], p + nwritten, count - nwritten, 0);
                if (res <= 0) {
                    break;
                }
                nwritten += res;
            }

            /** 如果没有完全写入，标记失败状态，并统计总共失败的个数 */
            if (nwritten != count) {
                this->states[i] = EIO;
                broken++;
            }
        }

        /** 在本轮写入完之后，查看所有的socket是否全失败了。如果全失败了，返回0 */
        if (broken == this->fds.size()) {
            return -1;
        }

        /** 本次发送完毕处理 */
        p += count;
        length -= count;
        pos += count;
    }

    /** 清空缓存 */
    this->buf.clear();

    return 1;
}

int SocketFio::getState(int idx) {
    if (idx > this->states.size()) {
        return -1;
    }

    return this->states[idx];
}

/** there is no need to read */
size_t SocketFio::baseread(void *buf, size_t readLen) {
    return 0;
}

size_t SocketFio::basewrite(const void *buf, size_t len) {
    if (NULL == buf || 0 == len) {
        return 0;
    }

    /** append */
    this->buf.append((char*)buf, len);

    /** If the length of buf is greater than PROTO_IOBUF_LEN, we need flush it to sockets */
    if (this->buf.length() > PROTO_IOBUF_LEN) {
        this->flush();
    }

    return 1;
}

void SocketFio::setFds(const std::vector<int> &fds) {
    this->fds = fds;
}

void SocketFio::initStates(int size) {
    this->states.resize(size);
    for (int i = 0; i < this->states.size(); i++) {
        this->states[i] = 0;
    }
}
