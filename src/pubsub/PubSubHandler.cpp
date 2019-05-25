//
// Created by levy on 2019/5/18.
//

#include "PubSubHandler.h"

PubSubHandler::PubSubHandler(const AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;
}

PubSubHandler::~PubSubHandler() {
    this->channels.clear();
    this->patterns.clear();
}

int PubSubHandler::publishMessage(const std::string &channel, const std::string &message) {
    int receivers = 0;

    /** 找到该channel的clients，并逐一发送消息 */
    std::map<std::string, std::list<std::shared_ptr<AbstractFlyClient>>*>::const_iterator citr = this->channels.find(channel);
    if (citr != this->channels.end()) {
        for (auto item : *(citr->second)) {
            this->addReplyPubsubMessage(item, channel, message);
            receivers++;
        }
    }

    /** 对模式订阅flyclient发送消息 */
    for (auto item : this->patterns) {
        if (miscTool->stringmatch(item->pattern, channel, false)) {
            this->addReplyPubsubMessage(item->flyClient, channel, message);
            receivers++;
        }
    }

    return receivers;
}

void PubSubHandler::addReplyPubsubMessage(std::shared_ptr<AbstractFlyClient> flyClient,
                                          const std::string& channel,
                                          const std::string& message) {
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getMbulkHeader(3));
    flyClient->addReplyBulkString(channel);
    flyClient->addReplyBulkString(message);
}

void PubSubHandler::addReplyPubsubPatternMessage(std::shared_ptr<AbstractFlyClient> flyClient,
                                                 const std::string& pattern,
                                                 const std::string& channel,
                                                 const std::string& message) {
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getMbulkHeader(4));
    flyClient->addReplyBulkString(pattern);
    flyClient->addReplyBulkString(channel);
    flyClient->addReplyBulkString(message);
}

/** 向客户端发送订阅通知 */
void PubSubHandler::addReplyPubsubSubscribed(std::shared_ptr<AbstractFlyClient> flyClient,
                                             const std::string& channel) {
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getMbulkHeader(3));
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getSubscribebulk());
    flyClient->addReplyBulkString(channel);
    flyClient->addReplyLongLong(flyClient->getSubscriptionsCount());
}

/** 向客户端发送取消订阅通知 */
void PubSubHandler::addReplyPubsubUnsubscribed(std::shared_ptr<AbstractFlyClient> flyClient,
                                               const std::string& channel) {
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getMbulkHeader(3));
    flyClient->addReplyFlyStringObj(coordinator->getSharedObjects()->getUnsubscribebulk());
    flyClient->addReplyBulkString(channel);
    flyClient->addReplyLongLong(flyClient->getSubscriptionsCount());
}

bool PubSubHandler::listMatchPubsubPattern(PubSubPattern *pa, PubSubPattern *pb) {
    /** 两者都为null */
    if (NULL == pa && NULL == pb) {
        return true;
    }

    /** 一个为null，另一个不为null */
    if (NULL == pa || NULL == pb) {
        return false;
    }

    return (pa->flyClient == pb->flyClient) && (0 ==pa->pattern.compare(pb->pattern));
}

int PubSubHandler::subscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient,
                                    const std::string& channel) {
    int retval = 0;
    if (flyClient->addChannel(channel)) {
        retval = 1;
        std::list<std::shared_ptr<AbstractFlyClient>> *clients = NULL;
        if (this->channels.find(channel) == this->channels.end()) {
            clients = new std::list<std::shared_ptr<AbstractFlyClient>>();
            this->channels[channel] = clients;
        } else {
            clients = this->channels.at(channel);
        }

        clients->push_back(flyClient);
    }

    /** 向客户端发送订阅通知 */
    this->addReplyPubsubSubscribed(flyClient, channel);

    return retval;
}

int PubSubHandler::unsubscribeChannel(std::shared_ptr<AbstractFlyClient> flyClient,
                                      const std::string& channel,
                                      bool notify) {
    int retval = 0;
    if (flyClient->delChannel(channel)) {
        retval = 1;
        if (this->channels.find(channel) != this->channels.end()) {
            /** 找到该channel的clients列表 */
            std::list<std::shared_ptr<AbstractFlyClient>> *clients = this->channels[channel];

            /** 在clients列表中找到flyClient并删除 */
            std::list<std::shared_ptr<AbstractFlyClient>>::iterator iter = clients->begin();
            for (iter; iter != clients->end(); iter++) {
                if (*iter == flyClient) {
                    /** 删除该flyClient */
                    clients->erase(iter);
                    /** 如果删除该flyClient后clients列表为空，则直接从channels中清除该channel */
                    if (0 == clients->size()) {
                        this->channels.erase(channel);
                    }
                }
            }
        }
    }

    /** 发送通知 */
    if (notify) {
        this->addReplyPubsubUnsubscribed(flyClient, channel);
    }

    return retval;
}

int PubSubHandler::unsubscribeAllChannels(std::shared_ptr<AbstractFlyClient> flyClient,
                                          bool notify) {
    const std::map<const std::string, void *> &allChannels = flyClient->getChannels();
    int count = 0;
    for (auto item : allChannels) {
        count += this->unsubscribeChannel(flyClient, item.first, notify);
    }

    /** 如果取消的订阅的数量为0（证明上述for循环没有发出任何消息给flyclient），同样需要发布取消订阅消息 */
    if (notify && 0 == count) {
        this->addReplyPubsubUnsubscribed(flyClient, getNullReply());
    }
    return count;
}

int PubSubHandler::subscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                                    const std::string &pattern) {
    int retval = 0;
    const std::list<const std::string> &allPatterns = flyClient->getPatterns();
    std::list<const std::string>::const_iterator iter = std::find(allPatterns.begin(), allPatterns.end(), pattern);
    if (iter == allPatterns.end()) {
        retval = 1;
        /** 将pattern string加入到flyclient中 */
        flyClient->addPattern(pattern);

        /** 生成一个PubSubPattern对象放入this->patterns中 */
        std::shared_ptr<PubSubPattern> pubsubPattern = std::shared_ptr<PubSubPattern>(new PubSubPattern(flyClient, pattern));
        this->patterns.push_back(pubsubPattern);
    }

    /** 向flyclient发送订阅通知 */
    this->addReplyPubsubSubscribed(flyClient, pattern);

    return retval;
}

int PubSubHandler::unsubscribePattern(std::shared_ptr<AbstractFlyClient> flyClient,
                                      const std::string &pattern,
                                      bool notify) {
    int retval = 0;
    const std::list<const std::string> &allPatterns = flyClient->getPatterns();
    std::list<const std::string>::const_iterator iter = std::find(allPatterns.begin(), allPatterns.end(), pattern);
    if (iter != allPatterns.end()) {
        retval = 1;

        /** 将pattern string从flyclient中删除 */
        flyClient->delPattern(pattern);

        /** 从this->patterns中查找并删除 */
        std::list<std::shared_ptr<PubSubPattern>>::const_iterator iter = this->patterns.begin();
        for (iter; iter != this->patterns.end(); iter++) {
            if (iter->get()->pattern.compare(pattern) && iter->get()->flyClient == flyClient) {
                this->patterns.erase(iter);
                break;
            }
        }
    }

    /** 发送取消订阅通知 */
    if (notify) {
        this->addReplyPubsubUnsubscribed(flyClient, pattern);
    }

    return retval;
}

int PubSubHandler::unsubscribeAllPatterns(std::shared_ptr<AbstractFlyClient> flyClient,
                                          bool notify) {
    const std::list<const std::string> &allPatterns = flyClient->getPatterns();
    int count = 0;
    for (auto item : allPatterns) {
        count += this->unsubscribePattern(flyClient, item, notify);
    }

    /** 如果取消的模式订阅的数量为0（证明上述for循环没有发出任何消息给flyclient），同样需要发布取消订阅消息 */
    if (notify && 0 == count) {
        this->addReplyPubsubUnsubscribed(flyClient, getNullReply());
    }
}

std::string PubSubHandler::getNullReply() {
    return "$-1\r\n";
}

