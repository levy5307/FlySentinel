//
// Created by levy on 2019/5/18.
//

#ifndef FLYDB_PUBSUBHANDLER_H
#define FLYDB_PUBSUBHANDLER_H

#include <string>
#include <list>
#include <map>
#include "../coordinator/interface/AbstractPubSubHandler.h"
#include "PubSubDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"

class PubSubHandler : public AbstractPubSubHandler {
public:
    PubSubHandler(const AbstractCoordinator *coordinator);
    ~PubSubHandler();
    /** 
     * 发送消息
     **/
    int publishMessage(const std::string &channel, const std::string &message);

    /**
     * 普通订阅
     **/
    /** 客户端订阅频道 */
    int subscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient,
                         const std::string& channel);
    /** 客户端取消订阅频道 */
    int unsubscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient,
                           const std::string& channel,
                           bool notify);
    /** 取消订阅该客户端的所有频道 */
    int unsubscribeAllChannels(std::shared_ptr<AbstractFlyClient> flyClient, bool notify);

    /**
     * 模式订阅
     **/
    /** 客户端订阅模式频道 */
    int subscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                         const std::string &pattern);
    /** 客户端取消订阅模式频道 */
    int unsubscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                           const std::string &pattern,
                           bool notify);
    /** 取消订阅该客户端的所有模式订阅频道 */
    int unsubscribeAllPatterns(std::shared_ptr<AbstractFlyClient> flyClient, bool notify);

private:
    void addReplyPubsubMessage(std::shared_ptr<AbstractFlyClient> flyClient,
                               const std::string& channel,
                               const std::string& message);
    void addReplyPubsubPatternMessage(std::shared_ptr<AbstractFlyClient> flyClient,
                                      const std::string& pat,
                                      const std::string& channel,
                                      const std::string& message);
    /** 向客户端发送(模式)订阅通知 */
    void addReplyPubsubSubscribed(std::shared_ptr<AbstractFlyClient> flyClient,
                                  const std::string& channel);
    /** 向客户端发送取消(模式)订阅通知 */
    void addReplyPubsubUnsubscribed(std::shared_ptr<AbstractFlyClient> flyClient,
                                    const std::string& channel);
    bool listMatchPubsubPattern(PubSubPattern *pa, PubSubPattern *pb);
    std::string getNullReply();

private:
    std::map<std::string, std::list<std::shared_ptr<AbstractFlyClient>>*> channels;
    std::list<std::shared_ptr<PubSubPattern>> patterns;
    const AbstractCoordinator *coordinator;
};


#endif //FLYDB_PUBSUBHANDLER_H
