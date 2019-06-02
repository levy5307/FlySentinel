//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_INSTANCELINK_H
#define FLYSENTINEL_INSTANCELINK_H

#include <cstdint>
#include "../coordinator/interface/AbstractInstanceLink.h"
#include "../asyncEvent/convert.h"

/**
 * 只对FlyInstance中的引用使用智能指针，这样智能指针的引用计数代表共享的FlyInstance数量，
 * 其他地方的引用不要使用
 **/
class InstanceLink : public AbstractInstanceLink {
public:
    InstanceLink();
    ~InstanceLink();
    void closeConnection(std::shared_ptr<redisAsyncContext> context);
    const std::shared_ptr<redisAsyncContext> &getCommandContext() const;
    const std::shared_ptr<redisAsyncContext> &getPubsubContext() const;
    void decreasePendingCommands();

private:
    bool disconnected = true;                                   // true-需要重连连接
    int pendingCommands = 0;                                    // 等待响应的命令数量
    std::shared_ptr<redisAsyncContext> commandContext = NULL;
    std::shared_ptr<redisAsyncContext> pubsubContext = NULL;
    uint64_t ccConnTime = 0;                                    // cc连接时间
    uint64_t pcConntime = 0;                                    // pc连接时间
    uint64_t pcLastActivity = 0;                                // 最后一次收到消息的时间
    uint64_t lastAvailTime;                                     // 上次可用时间
    uint64_t actPingTime;                                       // 距离上次接收到pong以来，最早的一次发送ping的时间。
                                                                // 如果接收到pong，该值会被设置为0。如果当前值为0并且发送了ping，则该值被设置为当前时间
    uint64_t lastPingTime = 0;                                  // 最后一次发送ping的时间
    uint64_t lastPongTime;                                      // 最后一次接收pong的时间
    uint64_t lastReconnTime = 0;                                // 最后一次进行重连的时间
};

#endif //FLYSENTINEL_INSTANCELINK_H
