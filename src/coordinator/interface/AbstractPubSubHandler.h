//
// Created by levy on 2019/5/18.
//

#ifndef FLYSENTINEL_ABSTRACTPUBSUBHANDLER_H
#define FLYSENTINEL_ABSTRACTPUBSUBHANDLER_H

class AbstractFlyClient;

class AbstractPubSubHandler {
public:
    virtual ~AbstractPubSubHandler() {};

    /**
     * 发送消息
     **/
    virtual int publishMessage(const std::string &channel, const std::string &message) = 0;

    /**
     * 普通订阅
     **/
    /** 客户端订阅频道 */
    virtual int subscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient, const std::string& channel) = 0;
    /** 客户端取消订阅频道 */
    virtual int unsubscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient,
                           const std::string& channel,
                           bool notify) = 0;
    /** 取消订阅该客户端的所有频道 */
    virtual int unsubscribeAllChannels(std::shared_ptr<AbstractFlyClient> flyClient, bool notify) = 0;

    /**
     * 模式订阅
     **/
    /** 客户端订阅模式频道 */
    virtual int subscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                         const std::string &pattern) = 0;
    /** 客户端取消订阅模式频道 */
    virtual int unsubscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                           const std::string &pattern,
                           bool notify) = 0;
    /** 取消订阅该客户端的所有模式订阅频道 */
    virtual int unsubscribeAllPatterns(std::shared_ptr<AbstractFlyClient> flyClient, bool notify) = 0;
};

#endif //FLYSENTINEL_ABSTRACTPUBSUBHANDLER_H
