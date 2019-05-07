//
// Created by levy on 2019/5/7.
//

#include <zconf.h>
#include <iostream>
#include "AbstractFlyServer.h"
#include "../../def.h"
#include "../../flyClient/FlyClient.h"

AbstractFlyServer::AbstractFlyServer(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;

    // init command table
    this->commandTable = new CommandTable(coordinator);

    // 设置最大客户端数量
    setMaxClientLimit();

    // serverCron运行频率
    this->hz = CONFIG_CRON_HZ;
    this->neterr = new char[NET_ERR_LEN];

    // 拒绝连接次数设置为0
    this->statRejectedConn = 0;
    pthread_mutex_init(&this->nextClientIdMutex, NULL);

    // 当前时间
    this->nowt = time(NULL);
    this->clientMaxQuerybufLen = PROTO_MAX_QUERYBUF_LEN;
}

void AbstractFlyServer::init(ConfigCache *configCache) {
    // 从configCache获取参数
    this->loadFromConfig(configCache);

    // 打开监听socket，用于监听用户命令
    this->listenToPort();

    // 打开Unix domain socket
    if (NULL != this->unixsocket) {
        unlink(this->unixsocket);       // 如果存在，则删除unixsocket文件
        this->usfd = this->coordinator->getNetHandler()->unixServer(
                this->neterr,
                this->unixsocket,
                this->unixsocketperm,
                this->tcpBacklog);
        if (-1 == this->usfd) {
            std::cout << "Opening Unix Domain Socket: " << this->neterr << std::endl;
            exit(1);
        }
        this->coordinator->getNetHandler()->setBlock(NULL, this->usfd, 0);
    }

    // 创建定时任务，用于创建客户端连接
    for (auto fd : this->ipfd) {
        if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                fd, ES_READABLE, acceptTcpHandler, nullptr)) {
            exit(1);
        }
    }

    this->logHandler = logFactory->getLogger();
    return;
}

int AbstractFlyServer::handleClientsWithPendingWrites() {
    int pendingCount = this->clientsPendingWrite.size();
    if (0 == pendingCount) {
        return 0;
    }

    std::list<std::shared_ptr<AbstractFlyClient> >::iterator iter = this->clientsPendingWrite.begin();
    for (; iter != this->clientsPendingWrite.end(); iter++) {
        // 先清除标记，清空了该标记才回保证该客户端再次加入到clientsPendingWrite里；
        // 否则无法加入。也就无法处理其输出
        (*iter)->delFlag(CLIENT_PENDING_WRITE);

        // 先直接发送，如果发送不完，再创建文件事件异步发送
        if (-1 == (*iter)->writeToClient(0)) {
            continue;
        }

        // 异步发送
        if (!(*iter)->hasNoPending()) {
            if (-1 == this->coordinator->getEventLoop()->createFileEvent(
                    (*iter)->getFd(), ES_WRITABLE, sendReplyToClient, *iter)) {
                freeClientAsync(*iter);
            }
        }
    }

    /** 清空列表 */
    this->clientsPendingWrite.clear();

    /** 返回清空的client数量 */
    return pendingCount;
}

void AbstractFlyServer::freeClientAsync(int fd) {
    std::shared_ptr<AbstractFlyClient> flyClient = this->getFlyClient(fd);
    if (nullptr == flyClient) {
        return;
    }

    freeClientAsync(flyClient);
}

void AbstractFlyServer::freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient) {
    if (flyClient->getFlags() & CLIENT_CLOSE_ASAP) {
        return;
    }

    flyClient->setFlags(CLIENT_CLOSE_ASAP);
    this->clientsToClose.push_back(flyClient);
}

std::shared_ptr<AbstractFlyClient> AbstractFlyServer::getFlyClient(int fd) {
    if (this->clients.find(fd) != this->clients.end()) {
        return this->clients[fd];
    }

    return nullptr;
}

int AbstractFlyServer::getMaxClients() const {
    return this->maxClients;
}

int AbstractFlyServer::listenToPort() {
    int fd;
    // try to bind all to IPV4 and IPV6
    if (0 == this->bindAddrs.size()) {
        int success = 0;
        // try to set *(any address) to ipv6
        fd = this->coordinator->getNetHandler()->tcp6Server(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        // try to set *(any address) to ipv4
        fd = this->coordinator->getNetHandler()->tcpServer(
                this->neterr,
                this->port,
                NULL,
                this->tcpBacklog);
        if (fd != -1) {
            // set nonblock
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
            success++;
        }

        if (0 == success) {
            return -1;
        }
    } else {
        for (auto addr : this->bindAddrs) {
            // 如果是IPV6
            if (addr.find(":") != addr.npos) {
                fd = this->coordinator->getNetHandler()->tcp6Server(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            } else {
                fd = this->coordinator->getNetHandler()->tcpServer(
                        this->neterr,
                        this->port,
                        addr.c_str(),
                        this->tcpBacklog);
            }
            if (-1 == fd) {
                return -1;
            }
            this->coordinator->getNetHandler()->setBlock(NULL, fd, 0);
            this->ipfd.push_back(fd);
        }
    }

    return 1;
}

void AbstractFlyServer::loadFromConfig(ConfigCache *configCache) {
    this->bindAddrs = configCache->getBindAddrs();
    this->unixsocket = configCache->getUnixsocket();
    this->unixsocketperm = configCache->getUnixsocketperm();
    this->tcpKeepAlive = configCache->getTcpKeepAlive();
    this->port = configCache->getPort();
}

bool AbstractFlyServer::dealWithCommand(int fd) {
    std::shared_ptr<AbstractFlyClient> flyClient = this->getFlyClient(fd);
    if (nullptr == flyClient) {
        return false;
    }

    return this->commandTable->dealWithCommand(flyClient);
}

void AbstractFlyServer::setMaxClientLimit() {
    this->maxClients = CONFIG_DEFAULT_MAX_CLIENTS;
    int maxFiles = this->maxClients + CONFIG_MIN_RESERVED_FDS;
    rlimit limit;

    // 获取当前进程可打开的最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        // 如果获取失败, 按照进程中最大文件数量为1024计算(内核默认1024),
        // 重置maxClients
        this->maxClients = 1024 - CONFIG_MIN_RESERVED_FDS;
    } else {
        int softLimit = limit.rlim_cur;
        // 如果soft limit小于maxfiles, 则尝试增大soft limit, 并重置maxClients
        if (softLimit < maxFiles) {
            int curLimit = maxFiles;
            int decrStep = 16;

            // 逐步试探提高softlimit
            while (curLimit > softLimit) {
                limit.rlim_cur = curLimit;
                if (setrlimit(RLIMIT_NOFILE, &limit) != -1) {
                    break;
                }
                curLimit -= decrStep;
            }
            if (curLimit < softLimit) {
                curLimit = softLimit;
            }

            // 如果当前文件数量限制小于最小保留文件数，程序退出
            if (curLimit <= CONFIG_MIN_RESERVED_FDS) {
                exit(1);
            }

            this->maxClients = curLimit - CONFIG_MIN_RESERVED_FDS;
        }
    }
}
