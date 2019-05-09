//
// Created by levy on 2019/5/5.
//

#ifndef FLYSENTINEL_INSTANCELINK_H
#define FLYSENTINEL_INSTANCELINK_H

#include <cstdint>
#include "../coordinator/interface/AbstractInstanceLink.h"

class InstanceLink : public AbstractInstanceLink {
private:
    bool disconnected;              // true-需要重连连接
    int pendingCommands;            // 等待响应的命令数量
    uint64_t ccConnTime;            // cc连接时间
    uint64_t pcConntime;            // pc连接时间
    uint64_t pcLastActivity;        // 最后一次收到消息的时间
    uint64_t lastAvailTime;         // 上次可用时间
    uint64_t actPingTime;           // 距离上次接收到pong以来，最早的一次发送ping的时间。
                                    // 如果接收到pong，该值会被设置为0。如果当前值为0并且发送了ping，则该值被设置为当前时间
    uint64_t lastPingTime;          // 最后一次发送ping的时间
    uint64_t lastPongTime;          // 最后一次接收pong的时间
    uint64_t lastReconnTime;        // 最后一次进行重连的时间
};


#endif //FLYSENTINEL_INSTANCELINK_H
