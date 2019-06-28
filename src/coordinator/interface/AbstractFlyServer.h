//
// Created by levy on 2019/6/12.
//

#ifndef FLYSENTINEL_ABSTRACTFLYSERVER_H
#define FLYSENTINEL_ABSTRACTFLYSERVER_H

#include <cstddef>
#include <cstdint>
#include "AbstractFlyClient.h"
#include "AbstractFlyInstance.h"

class AbstractFlyServer {
public:
    virtual ~AbstractFlyServer() {};

    /**
     * sentinel server interface
     **/
    virtual void sendEvent(int level, char *type, std::shared_ptr<AbstractFlyInstance> flyInstance, const char *fmt, ...) = 0;
    virtual void generateInitMonitorEvents() = 0;
    virtual int tryConnectionSharing(std::shared_ptr<AbstractFlyInstance> flyInstance) = 0;
    virtual void flushConfig() = 0;
    /** 将master的downAfterPeriod设置给与该master相连的所有sentinels和slaves */
    virtual void propagateDownAfterPeriod(std::shared_ptr<AbstractFlyInstance> master) = 0;
    virtual void refreshInstanceInfo(AbstractFlyInstance* flyInstance, const std::string &info) = 0;
    virtual const std::string &getAnnounceIP() const = 0;
    virtual void setAnnounceIP(const std::string &announceIP) = 0;
    virtual int getAnnouncePort() const = 0;
    virtual void setAnnouncePort(int announcePort) = 0;
    virtual const char *getMyid() const = 0;
    virtual uint64_t getCurrentEpoch() const = 0;
    virtual void processHelloMessage(std::string &hello) = 0;

    /**
     * general server interface
     **/
    /** network */
    virtual size_t getClientMaxQuerybufLen() const = 0;
    virtual int64_t getStatNetInputBytes() const = 0;
    virtual void addToStatNetInputBytes(int64_t size) = 0;
    virtual const std::vector<int> &getIpfd() const = 0;
    virtual int getPort() const = 0;
    virtual void setPort(int port) = 0;

    /** clients */
    virtual void addToClientsPendingToWrite(int fd) = 0;
    virtual int handleClientsWithPendingWrites() = 0;
    virtual std::shared_ptr<AbstractFlyClient> createClient(int fd) = 0;
    virtual int freeClient(std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual void freeClientAsync(std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual void freeClientAsync(int fd) = 0;
    virtual int getMaxClients() const = 0;
    virtual void unlinkClient(std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual void linkClient(std::shared_ptr<AbstractFlyClient> flyClient) = 0;
    virtual std::shared_ptr<AbstractFlyClient> getFlyClient(int fd) = 0;
    virtual void freeClientsInAsyncFreeList() = 0;

    /** command */
    virtual bool dealWithCommand(int fd) = 0;

    /** cron loop */
    virtual int getHz() const = 0;
    virtual void setHz(int hz) = 0;
    virtual time_t getNowt() const = 0;
    virtual void setNowt(time_t nowt) = 0;
    virtual void addCronLoops() = 0;
    virtual uint64_t getCronLoops() const = 0;

    virtual void addReplyRedisInstances(std::shared_ptr<AbstractFlyClient> flyClient,
                                        std::map<std::string, std::shared_ptr<AbstractFlyInstance>> instanceMap) = 0;
};

#endif //FLYSENTINEL_ABSTRACTFLYSERVER_H
