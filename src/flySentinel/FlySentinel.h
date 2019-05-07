//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_FLYSENTINEL_H
#define FLYSENTINEL_FLYSENTINEL_H

#include <map>
#include <vector>
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../config/ConfigCache.h"

int serverCron(const AbstractCoordinator *coordinator, uint64_t id, void *clientData);

class FlySentinel : public AbstractFlyServer {
public:
    FlySentinel(const AbstractCoordinator *coordinator);
    void init(ConfigCache *configCache);
    int handleClientsWithPendingWrites();
    void freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient);
    void freeClientAsync(int fd);
    int getMaxClients() const;

private:
    std::shared_ptr<AbstractFlyClient> getFlyClient(int fd);
    int listenToPort();
    void loadFromConfig(ConfigCache *configCache);

    /**
     *  网络相关
     */
    int port;                                 // tcp listening port
    std::vector<int> ipfd;                    // TCP socket fd
    std::vector<std::string> bindAddrs;        // 绑定地址
    int tcpBacklog;                           // TCP listen() backlog
    char *neterr;                             // 网络error buffer
    const char *unixsocket;                   // UNIX socket path
    mode_t unixsocketperm;                    // UNIX socket permission
    int usfd;                                 // Unix socket file descriptor
    int64_t statNetInputBytes;                // 该server从网络获取的byte数量
    int tcpKeepAlive;

    /**
     * client相关
     */
    /** 最大可同时连接的client数量 */
    int maxClients;
    uint64_t nextClientId;
    pthread_mutex_t nextClientIdMutex;
    /** client列表 */
    std::map<int, std::shared_ptr<AbstractFlyClient> > clients;
    /** 需要install write handler */
    std::list<std::shared_ptr<AbstractFlyClient> > clientsPendingWrite;
    /** 异步关闭的client链表 */
    std::list<std::shared_ptr<AbstractFlyClient> > clientsToClose;
    /** 由于超过了maxclients而拒绝连接的次数 */
    uint64_t statRejectedConn;
    /** client buff最大长度 */
    size_t clientMaxQuerybufLen;
    uint64_t cronloops = 0;

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_FLYSENTINEL_H
