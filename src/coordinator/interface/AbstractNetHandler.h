//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_ABSTRACTNETHANDLER_H
#define FLYSENTINEL_ABSTRACTNETHANDLER_H

#include <sys/socket.h>

class AbstractNetHandler {
public:
    virtual ~AbstractNetHandler() {};
    
    virtual int setV6Only(char *err, int fd) = 0;

    virtual int setSendTimeout(char *err, int fd, long long ms) = 0;

    virtual int disableTcpNoDelay(char *err, int fd) = 0;

    virtual int enableTcpNoDelay(char *err, int fd) = 0;

    virtual int setSendBuffer(char *err, int fd, int buffsize) = 0;

    virtual int setTcpKeepAlive(char *err, int fd) = 0;

    virtual int resolve(char *err, char *host, char *ipbuf, size_t ipbuf_len) = 0;

    virtual int resolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len) = 0;

    virtual int createSocket(char *err, int domain) = 0;

    virtual int keepAlive(char *err, int fd, int interval) = 0;

    virtual int tcpConnect(char *err, char *addr, int port) = 0;

    virtual int tcpNonBlockConnect(char *err, char *addr, int port) = 0;

    virtual int tcpNonBlockBindConnect(char *err, char *addr, int port, char *source_addr) = 0;

    virtual int tcpNonBlockBestEffortBindConnect(char *err, const char *addr, int port, const char *source_addr) = 0;

    virtual int setListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog) = 0;

    virtual int unixServer(char *err, const char *path, mode_t perm, int backlog) = 0;

    virtual int tcpServer(char *err, int port, const char *bindaddr, int backlog) = 0;

    virtual int tcp6Server(char *err, int port, const char *bindaddr, int backlog) = 0;

    virtual int setBlock(char *err, int fd, int block) = 0;

    virtual int tcpAccept(char *err, int s, char *ip, size_t iplen, int *port) = 0;

    virtual int unixAccept(char *err, int s) = 0;

    virtual int wait(int fd, int mask, int millseconds) = 0;

    virtual ssize_t syncRead(int fd, char * ptr, int size, uint64_t timeout) = 0;

    virtual ssize_t syncReadLine(int fd, char *ptr, int size, uint64_t timeout) = 0;

    virtual ssize_t syncWrite(int fd, std::string str, uint64_t timeout) = 0;

    virtual int peerToString(int fd, char *ip, size_t iplen, int *port) = 0;
};

#endif //FLYSENTINEL_ABSTRACTNETHANDLER_H
