//
// Created by 赵立伟 on 2018/11/3.
//

#ifndef FLYDB_NETHANDLER_H
#define FLYDB_NETHANDLER_H

#include <sys/socket.h>
#include "../utils/MiscTool.h"
#include "../coordinator/interface/AbstractNetHandler.h"
#include "../coordinator/interface/AbstractLogHandler.h"
#include "../coordinator/interface/AbstractLogFactory.h"

class NetHandler : public AbstractNetHandler {
public:
    ~NetHandler();
    static NetHandler* getInstance();
    int setV6Only(char *err, int fd);
    int setSendTimeout(char *err, int fd, long long ms);
    /** val: 1-enbale no delay, 0-disable */
    int disableTcpNoDelay(char *err, int fd);
    int enableTcpNoDelay(char *err, int fd);
    int setSendBuffer(char *err, int fd, int buffsize);
    int setTcpKeepAlive(char *err, int fd);
    int resolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    int resolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);
    int createSocket(char *err, int domain);
    int keepAlive(char *err, int fd, int interval);
    int tcpConnect(char *err, char *addr, int port);
    int tcpNonBlockConnect(char *err, char *addr, int port);
    int tcpNonBlockBindConnect(char *err,
                               char *addr,
                               int port,
                               char *source_addr);
    int tcpNonBlockBestEffortBindConnect(char *err,
                                         const char *addr,
                                         int port,
                                         const char *source_addr);
    int setListen(char *err,
                  int s,
                  struct sockaddr *sa,
                  socklen_t len,
                  int backlog);
    int unixServer(char *err, const char *path, mode_t perm, int backlog);
    int tcpServer(char *err, int port, const char *bindaddr, int backlog);
    int tcp6Server(char *err, int port, const char *bindaddr, int backlog);
    int setBlock(char *err, int fd, int block);
    int tcpAccept(char *err, int s, char *ip, size_t iplen, int *port);
    int unixAccept(char *err, int s);
    int wait(int fd, int mask, int millseconds);
    ssize_t syncRead(int fd, char*ptr, int size, uint64_t timeout);
    ssize_t syncReadLine(int fd, char *ptr, int size, uint64_t timeout);
    ssize_t syncWrite(int fd, std::string str, uint64_t timeout);
    int peerToString(int fd, char *ip, size_t iplen, int *port);

private:
    NetHandler();
    void setError(char *err, const char *fmt, ...);
    int genericResolve(char *err,
                       char *host,
                       char *ipbuf,
                       size_t ipbuf_len,
                       int flags);
    int setReuseAddr(char *err, int fd);
    int tcpGenericConnect(char *err,
                          const char *addr,
                          int port,
                          const char *source_addr,
                          int flags);
    int tcpGenericServer(char *err,
                         int port,
                         const char *bindaddr,
                         int af,
                         int backlog);
    int tcpGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len);
    void dealError(int fd, struct addrinfo *servinfo);
    // val: 1-enbale no delay, 0-disable
    int setTcpNoDelay(char *err, int fd, int val);

    AbstractLogHandler *logHandler;
};

#endif //FLYDB_NETHANDLER_H
