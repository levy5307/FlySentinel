//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include <vector>
#include "CommandEntry.h"
#include "CommandTableDef.h"

/**
 * 命令列表：
 *     w-表示会修改db, 有该标志下，会将命令序列追加到aof文件中
 *     A-表示是访问db的key操作，此时在访问之前会判断该键是否已过期
 **/
std::vector<CommandEntry* > sentinelCommandTable = {
        new CommandEntry("ping",        pingCommand,        1, "tF",  0, NULL, 0, 0, 0, 0, 0),
        new CommandEntry("subscribe",   subscribeCommand,   2, "",    0, NULL, 0, 0, 0, 0, 0),
        new CommandEntry("unsubscribe", unsubscribeCommand, 1, "",    0, NULL, 0, 0, 0, 0, 0),
        new CommandEntry("psubscribe",  psubscribeCommand,  2, "",    0, NULL, 0, 0, 0, 0, 0),
        new CommandEntry("punsubscribe",punsubscribeCommand,1, "",    0, NULL, 0, 0, 0, 0, 0),
        new CommandEntry("publish",     publishCommand,     3, "",    0, NULL, 0, 0, 0, 0, 0)
};

CommandEntry::CommandEntry() {

}

CommandEntry::CommandEntry(commandProc proc, int flag) {
    this->proc = proc;
    this->flag = flag;
}

CommandEntry::CommandEntry(const char *name,
                           commandProc proc,
                           int arity,
                           const std::string &sflags,
                           int flag,
                           getKeysProc keysProc,
                           int firstKey,
                           int lastKey,
                           int keyStep,
                           uint64_t microseconds,
                           uint64_t calls) {
    this->name = name;
    this->proc = proc;
    this->arity = arity;
    this->sflags = sflags;
    this->flag = flag;
    this->keysProc = keysProc;
    this->firstKey = firstKey;
    this->lastKey = lastKey;
    this->keyStep = keyStep;
    this->microseconds = microseconds;
    this->calls = calls;
}

const char *CommandEntry::getName() const {
    return this->name;
}

void CommandEntry::setName(const char *name) {
    this->name = name;
}

commandProc CommandEntry::getProc() const {
    return this->proc;
}

void CommandEntry::setProc(commandProc proc) {
    this->proc = proc;
}

int CommandEntry::getArity() const {
    return this->arity;
}

void CommandEntry::setArity(int arity) {
    this->arity = arity;
}

const std::string &CommandEntry::getSflags() const {
    return this->sflags;
}

void CommandEntry::setSflags(const std::string &sflags) {
    this->sflags = sflags;
}

int CommandEntry::getFlag() const {
    return this->flag;
}

void CommandEntry::setFlag(int flag) {
    this->flag = flag;
}

void CommandEntry::addFlag(int flag) {
    this->flag |= flag;
}

bool CommandEntry::IsWrite() const {
    return this->flag & CMD_WRITE;
}

void CommandEntry::setKeysProc(getKeysProc proc) {
    this->keysProc = keysProc;
}

uint64_t CommandEntry::getMicroseconds() const {
    return this->microseconds;
}

void CommandEntry::setMicroseconds(uint64_t microseconds) {
    this->microseconds = microseconds;
}

uint64_t CommandEntry::getCalls() const {
    return this->calls;
}

void CommandEntry::setCalls(uint64_t calls) {
    this->calls = calls;
}

int CommandEntry::getFirstKey() const {
    return firstKey;
}

void CommandEntry::setFirstKey(int firstKey) {
    CommandEntry::firstKey = firstKey;
}

int CommandEntry::getLastKey() const {
    return lastKey;
}

void CommandEntry::setLastKey(int lastKey) {
    CommandEntry::lastKey = lastKey;
}

int CommandEntry::getKeyStep() const {
    return keyStep;
}

void CommandEntry::setKeyStep(int keyStep) {
    CommandEntry::keyStep = keyStep;
}

void pingCommand(const AbstractCoordinator* coordinator,
                 std::shared_ptr<AbstractFlyClient> flyClient) {
    std::shared_ptr<FlyObj> pongObj = coordinator->getSharedObjects()->getPong();
    flyClient->addReply(reinterpret_cast<std::string *>(pongObj->getPtr())->c_str());
}

void subscribeCommand(const AbstractCoordinator* coordinator,
                      std::shared_ptr<AbstractFlyClient> flyClient) {
    for (int i = 0; i < flyClient->getArgc(); i++) {
        std::string *argvi = reinterpret_cast<std::string*>(flyClient->getArgv()[i]->getPtr());
        coordinator->getPubSubHandler()->subscribeChannel(flyClient, *argvi);
    }

    /** 设置客户端为Pub/Sub模式 */
    flyClient->addFlag(CLIENT_PUBSUB);
}

void unsubscribeCommand(const AbstractCoordinator* coordinator,
                        std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 如果只有一个unsubscribe，则说明是取消订阅全部channel */
    if (1 == flyClient->getArgc()) {
        coordinator->getPubSubHandler()->unsubscribeAllChannels(flyClient, true);
    } else {
        for (int i = 0; i < flyClient->getArgc(); i++) {
            std::string *argvi = reinterpret_cast<std::string*>(flyClient->getArgv()[i]->getPtr());
            coordinator->getPubSubHandler()->unsubscribeChannel(flyClient, *argvi, true);
        }
    }

    /** 如果当前订阅数量为0，则删除该client的Pub/Sub模式flag */
    if (0 == flyClient->getSubscriptionsCount()) {
        flyClient->delFlag(CLIENT_PUBSUB);
    }
}

void psubscribeCommand(const AbstractCoordinator* coordinator,
                       std::shared_ptr<AbstractFlyClient> flyClient) {
    for (int i = 0; i < flyClient->getArgc(); i++) {
        std::string *argvi = reinterpret_cast<std::string*>(flyClient->getArgv()[i]->getPtr());
        coordinator->getPubSubHandler()->subscribePattern(flyClient, *argvi);
    }

    /** 设置客户端为Pub/Sub模式 */
    flyClient->addFlag(CLIENT_PUBSUB);
}

void punsubscribeCommand(const AbstractCoordinator* coordinator,
                         std::shared_ptr<AbstractFlyClient> flyClient) {
    /** 如果只有一个unsubscribe，则说明是取消订阅全部patterns */
    if (1 == flyClient->getArgc()) {
        coordinator->getPubSubHandler()->unsubscribeAllPatterns(flyClient, true);
    } else {
        for (int i = 0; i < flyClient->getArgc(); i++) {
            std::string *argvi = reinterpret_cast<std::string*>(flyClient->getArgv()[i]->getPtr());
            coordinator->getPubSubHandler()->unsubscribePattern(flyClient, *argvi, true);
        }
    }

    /** 如果当前订阅数量为0，则删除该client的Pub/Sub模式flag */
    if (0 == flyClient->getSubscriptionsCount()) {
        flyClient->delFlag(CLIENT_PUBSUB);
    }
}

void publishCommand(const AbstractCoordinator* coordinator,
                    std::shared_ptr<AbstractFlyClient> flyClient) {
    std::string *argv1 = reinterpret_cast<std::string*>(flyClient->getArgv()[1]->getPtr());
    std::string *argv2 = reinterpret_cast<std::string*>(flyClient->getArgv()[2]->getPtr());
    int receivers = coordinator->getPubSubHandler()->publishMessage(*argv1, *argv2);

    // todo: 同步操作

    /** 将通知到的receivers数量发送给client */
    flyClient->addReplyLongLong(receivers);
}
