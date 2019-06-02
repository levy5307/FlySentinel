//
// Created by 赵立伟 on 2018/11/3.
//

#include <sys/socket.h>
#include <sys/time.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <cstdarg>
#include <cstdio>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <fcntl.h>
#include <array>
#include <sys/un.h>
#include <sys/stat.h>
#include <iostream>
#include <poll.h>
#include "NetHandler.h"
#include "NetDef.h"
#include "../utils/MiscTool.h"
#include "../event/EventDef.h"

extern AbstractLogFactory *logFactory;
extern MiscTool *miscTool;

NetHandler::NetHandler() {
    this->logHandler = logFactory->getLogger();
}

NetHandler::~NetHandler() {

}

NetHandler* NetHandler::getInstance() {
    static NetHandler* instance;
    if (nullptr == instance) {
        instance = new NetHandler();
    }

    return instance;
}

int NetHandler::setV6Only(char *err, int fd) {
    int yes = 1;

    if (-1 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes))) {
        setError(err, "setsockopt IPV6_V6ONLY: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setSendTimeout(char *err,
                               int fd,
                               long long ms) {
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = ms % 1000;

    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))) {
        setError(err, "setsockopt SO_SNDTIMEO: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::disableTcpNoDelay(char *err, int fd) {
    return this->setTcpNoDelay(err, fd, 0);
}

int NetHandler::enableTcpNoDelay(char *err, int fd) {
    return this->setTcpNoDelay(err, fd, 1);
}

int NetHandler::setTcpNoDelay(char *err, int fd, int val) {
    if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val))) {
        setError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setSendBuffer(char *err, int fd, int buffsize) {
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                         &buffsize, sizeof(buffsize))) {
        setError(err, "setsockopt SO_SNDBUF: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setTcpKeepAlive(char *err, int fd) {
    int yes;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes))) {
        setError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::resolve(char *err,
                        char *host,
                        char *ipbuf,
                        size_t ipbuf_len) {
    return genericResolve(err, host, ipbuf, ipbuf_len, NET_NONE);
}

int NetHandler::resolveIP(char *err,
                          char *host,
                          char *ipbuf,
                          size_t ipbuf_len) {
    return genericResolve(err, host, ipbuf, ipbuf_len, NET_IP_ONLY);
}

int NetHandler::createSocket(char *err, int domain) {
    int sock;
    if ((sock = socket(domain, SOCK_STREAM, 0)) == -1) {
        setError(err, "creating socket: %s", strerror(errno));
        return -1;
    }

    if (-1 == setReuseAddr(err, sock)) {
        close(sock);
        return -1;
    }
    return sock;
}

int NetHandler::keepAlive(char *err, int fd, int interval) {
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
        setError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return -1;
    }

#ifdef __linux__
    // 这些都是linux特有api，所以只有linux环境下才会执行如下步骤
    // 开始首次keepAlive探测前的TCP空闲时间
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
        return -1;
    }

    // 两次keepAlive探测间的间隔时间
    val = interval / 3;
    if (val == 0) {
        val = 1;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
        return -1;
    }

    // 判定断开前的keepAlive探测次数
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
        setError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
        return -1;
    }
#else
    ((void) interval); /* Avoid unused var warning for non Linux systems. */
#endif

    return 1;
}

int NetHandler::tcpConnect(char *err, char *addr, int port) {
    return tcpGenericConnect(err, addr, port, nullptr, NET_CONNECT_NONE);
}

int NetHandler::tcpNonBlockConnect(char *err, char *addr, int port) {
    return tcpGenericConnect(err, addr, port, nullptr, NET_CONNECT_NONBLOCK);
}

int NetHandler::tcpNonBlockBindConnect(char *err,
                                       char *addr,
                                       int port,
                                       char *source_addr) {
    return tcpGenericConnect(err, addr, port, source_addr,
                             NET_CONNECT_NONBLOCK);
}

int NetHandler::tcpNonBlockBestEffortBindConnect(char *err,
                                                 const char *addr,
                                                 int port,
                                                 const char *source_addr) {
    return tcpGenericConnect(err, addr, port, source_addr,
                             NET_CONNECT_NONBLOCK | NET_CONNECT_BE_BINDING);
}

int NetHandler::tcpAccept(char *err,
                          int s,
                          char *ip,
                          size_t iplen,
                          int *port) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);

    // accept
    int fd = tcpGenericAccept(err, s, &sa, &salen);
    if(-1 == fd) {
        return -1;
    }

    // analysis the ip and port
    if (AF_INET == sa.sa_family) {
        struct sockaddr_in *sin = (struct sockaddr_in *) &sa;
        if (nullptr != ip) {
            inet_ntop(AF_INET, (void *)&sin->sin_addr, ip, iplen);
        }
        if (nullptr != port) {
            *port = ntohs(sin->sin_port);
        }
    } else {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *) &sa;
        if (nullptr != ip) {
            inet_ntop(AF_INET6, (void*)&sin->sin6_addr, ip, iplen);
        }
        if (nullptr != port) {
            *port = ntohs(sin->sin6_port);
        }
    }

    return fd;
}

int NetHandler::unixAccept(char *err, int s) {
    struct sockaddr sa;
    socklen_t salen = sizeof(sa);
    return tcpGenericAccept(err, s, &sa, &salen);
}

int NetHandler::genericResolve(char *err,
                               char *host,
                               char *ipbuf,
                               size_t ipbuf_len,
                               int flags) {
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, 0, sizeof(hints));
    if (flags & NET_IP_ONLY) hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, nullptr, &hints, &info)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }

    freeaddrinfo(info);
    return 1;
}

int NetHandler::setReuseAddr(char *err, int fd) {
    int yes = 1;

    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
        setError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return -1;
    }
    return 1;
}

int NetHandler::setListen(char *err,
                          int s,
                          struct sockaddr *sa,
                          socklen_t len,
                          int backlog) {
    if (bind(s, sa, len) == -1) {
        setError(err, "bind: %s", strerror(errno));
        close(s);
        return -1;
    }

    if (listen(s, backlog) == -1) {
        setError(err, "listen: %s", strerror(errno));
        close(s);
        return -1;
    }
    return 1;
}

int NetHandler::tcpGenericConnect(char *err,
                                  const char *addr,
                                  int port,
                                  const char *source_addr,
                                  int flags) {
    int s = -1, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /** 获取连接(目标)地址的所有地址信息 */
    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }

    /** 逐步尝试所有目标地址信息，直到有一个可以连接上 */
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        /**
         * 创建并connect socket, 如果这个servinfo失败了，则尝试下一个
         **/
        if (-1 == (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
            continue;
        }

        // reuse address
        if (-1 == setReuseAddr(err, s)) {
            goto error;

        }
        // set block
        if (flags & NET_CONNECT_NONBLOCK && -1 == setBlock(err, s, 0)) {
            goto error;
        }

        /** 如果设置了源地址，则绑定源地址；否则，使用系统默认绑定 */
        if (nullptr != source_addr) {
            int bound = 0;
            // 获得source地址信息
            if ((rv = getaddrinfo(source_addr,
                    nullptr, &hints, &bservinfo)) != 0) {
                setError(err, "%s", gai_strerror(rv));
                goto error;
            }

            // 绑定地址
            for (b = bservinfo; b != nullptr; b = b->ai_next) {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (!bound) {
                setError(err, "bind: %s", strerror(errno));
                goto error;
            }
        }

        /** connect连接服务器 */
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & NET_CONNECT_NONBLOCK) {
                goto end;
            }
            // 连接失败，则继续使用下一个地址尝试连接
            close(s);
            s = -1;
            continue;
        }

        // conect成功
        goto end;
    }
    if (p == nullptr) {
        setError(err, "creating socket: %s", strerror(errno));
    }

error:
    if (s != -1) {
        close(s);
        s = -1;
    }

end:
    freeaddrinfo(servinfo);

    /**
     * 处理best effort binding: 如果传递了source_addr,
     * 但是创建socket失败，则使用不传递source_addr的方式重试一次
     */
    if (-1 == s && source_addr && (flags & NET_CONNECT_BE_BINDING)) {
        return tcpGenericConnect(err, addr, port, nullptr, flags);
    } else {
        return s;
    }
}

int NetHandler::tcpServer(char *err,
                          int port,
                          const char *bindaddr,
                          int backlog) {
    return tcpGenericServer(err, port, bindaddr, AF_INET, backlog);
}

int NetHandler::tcp6Server(char *err,
                           int port,
                           const char *bindaddr,
                           int backlog) {
    return tcpGenericServer(err, port, bindaddr, AF_INET6, backlog);
}

int NetHandler::tcpGenericServer(char *err,
                                 int port,
                                 const char *bindaddr,
                                 int af,
                                 int backlog) {
    int s = -1, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints, *servinfo, *p;

    snprintf(_port, sizeof(_port), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0) {
        setError(err, "%s", gai_strerror(rv));
        return -1;
    }
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        // set ipv6 only
        if (af == AF_INET6 && -1 == setV6Only(err, s)) {
            dealError(s, servinfo);
            return -1;
        }

        // reuse address
        if (-1 == setReuseAddr(err, s)) {
            dealError(s, servinfo);
            return -1;
        }

        // listen to s
        if (-1 == setListen(err, s, p->ai_addr, p->ai_addrlen, backlog)) {
            dealError(s, servinfo);
            return -1;
        }

        // 执行成功
        freeaddrinfo(servinfo);
        return s;
    }

    // 如果所有地址都执行失败，运行错误
    if (nullptr == p) {
        setError(err, "unable to bind socket, errno: %d", errno);
        dealError(s, servinfo);
        return -1;
    }
}

int NetHandler::unixServer(char *err,
                           const char *path,
                           mode_t perm,
                           int backlog) {
    int s;
    struct sockaddr_un sa;

    if (-1 == (s = createSocket(err, AF_LOCAL))) {
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (-1 == setListen(err, s, (struct sockaddr*)&sa, sizeof(sa), backlog)) {
        return -1;
    }
    if (perm) {
        chmod(sa.sun_path, perm);
    }

    return s;
}

int NetHandler::setBlock(char *err, int fd, int block) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        setError(err, "fcntl(F_GETFL): %s", strerror(errno));
        return -1;
    }

    if (block) {
        flags &= ~O_NONBLOCK;

    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) == -1) {
        setError(err, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return -1;
    }
    return 1;
}

void NetHandler::setError(char *err, const char *fmt, ...) {
    if (!err) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(err, NET_ERR_LEN, fmt, ap);
    va_end(ap);
}

void NetHandler::dealError(int fd, struct addrinfo *servinfo) {
    if (fd != -1) {
        close(fd);
    }

    freeaddrinfo(servinfo);
}

int NetHandler::tcpGenericAccept(char *err, int s,
                                 struct sockaddr *sa, socklen_t *len) {
    while (1) {
        int fd = accept(s, sa, len);
        if (-1 == fd) {
            if (EINTR == errno) {
                continue;
            } else {
                setError(err, "accept: %s", strerror(errno));
                return -1;
            }
        }

        return fd;
    }
}

int NetHandler::wait(int fd, int mask, int millseconds) {
    /** 初始化pfd */
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    if (mask & ES_READABLE) {
        pfd.events |= POLLIN;
    }
    if (mask & ES_WRITABLE) {
        pfd.events |= POLLOUT;
    }

    /**
     * 获取设备符状态：
     *      如果返回 = 0，表示没有设备符准备好
     *      如果返回 < 0, 表示出错
     **/
    int retval, retmask;
    if ((retval = poll(&pfd, 1, millseconds)) <= 0) {
        return retval;
    }

    /** 设置设备符可读可写状态mask并返回 */
    if ((pfd.revents & POLLIN) && (mask & ES_READABLE)) {
        retmask |= ES_READABLE;
    }
    if ((pfd.revents & POLLOUT) && (mask & ES_WRITABLE)) {
        retmask |= ES_WRITABLE;
    }
    return retmask;
}

ssize_t NetHandler::syncRead(int fd,
                             char *ptr,
                             int size,
                             uint64_t timeout) {
    if (0 == size) {
        return 0;
    }

    uint64_t start = miscTool->mstime(), span = 0;
    ssize_t readCount = 0;
    while ((span = miscTool->mstime() - start) < timeout) {
        /** read once */
        int readRes = 0;
        if ((readRes = read(fd, ptr + readCount, size - readCount)) <= 0) {
            if (-1 == readRes && EAGAIN == errno) {
                continue;
            }

            /** 读取到文件末尾 */
            if (0 == readRes) {
                return readCount;
            }

            return -1;
        }

        /** weather read all or not */
        readCount += readRes;
        if (readCount == size) {
            break;
        }

        /** wait fd readable */
        this->wait(fd, ES_READABLE, timeout - span);
    }

    return readCount;
}

ssize_t NetHandler::syncReadLine(int fd, char *ptr, int size, uint64_t timeout) {
    ssize_t nread = 0;

    /** 留出一个字节给'\0' */
    size--;

    /** 循环读取，每次读一个字节 */
    while (size > 0) {
        /** 读取一个字节 */
        char ch;
        int readOnce = this->syncRead(fd, &ch, 1, timeout);
        if (-1 == readOnce) { // read error
            return -1;
        } else if (0 == readOnce) { // read 0 byte
            *ptr = '\0';
            return 0;
        }

        /** 如果当前字节是'\n'，如果是则说明读完了一行。判断上一个字节是否为'\r', 是的话设置为'\0' */
        if ('\n' == ch) {
            if (nread > 0 && '\r' == *(ptr - 1)) {
                nread--;
                *(ptr - 1) = '\0';
            }
            return nread;
        } else {
            *(ptr++) = ch;
            *ptr = '\0';
            nread++;
        }
        size--;
    }

    return nread;
}

ssize_t NetHandler::syncWrite(int fd,
                              std::string str,
                              uint64_t timeout) {
    uint64_t start = miscTool->mstime(), span = 0;
    ssize_t totalWrite = 0;

    char *ptr = (char*)str.c_str();
    while ((span = miscTool->mstime() - start) < timeout) {
        int writeRes = 0;
        if ((writeRes = write(fd, ptr + totalWrite, str.length() - totalWrite)) <= 0) {
            /** if res is -1 and errno means read again, then continue */
            if (-1 == writeRes && EAGAIN == errno) {
                continue;
            }

            return -1;
        }

        /** if write all done, break and return */
        totalWrite += writeRes;
        if (writeRes == str.length()) {
            break;
        }

        /** wait fd writable */
        this->wait(fd, ES_WRITABLE, timeout - span);
    }

    return totalWrite;
}

int NetHandler::peerToString(int fd, char *ip, size_t iplen, int *port) {
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if (nullptr == ip || nullptr == port) {
        this->logHandler->logWarning("ip and port can`t be nullptr!");
        return -1;
    }

    /** 获取socket的对方地址 */
    if (0 == iplen || -1 == getpeername(fd, (struct sockaddr*)&sa, &salen)) {
        if (iplen >= 2) {
            ip[0] = '?';
            ip[1] = '\0';
        } else if (1 == iplen) {
            ip[0] = '\0';
        }

        *port = 0;
        return -1;
    }

    /** 根据不同的协议族解析ip和port */
    if (AF_INET == sa.ss_family) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        inet_ntop(AF_INET,
                  reinterpret_cast<void*>(&(s->sin_addr)),
                  ip,
                  iplen);
        *port = ntohs(s->sin_port);
    } else if (AF_INET6 == sa.ss_family) {
        struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)&sa;
        inet_ntop(AF_INET6,
                  reinterpret_cast<void*>(&(s6->sin6_addr)),
                  ip,
                  iplen);
        *port = ntohs(s6->sin6_port);
    } else if (AF_UNIX == sa.ss_family) {
        strncpy(ip, "/unixsocket", iplen);
        *port = 0;
    } else {
        if (iplen >= 2) {
            ip[0] = '?';
            ip[1] = '\0';
        } else if (1 == iplen) {
            ip[0] = '\0';
        }

        *port = 0;
        return -1;
    }

    return 1;
}

std::string NetHandler::formatAddr(char *ip, int port) {
    char buf[NET_PEER_ID_LEN];
    snprintf(buf, NET_PEER_ID_LEN, "[%s]:%d", ip, port);
    return buf;
}
