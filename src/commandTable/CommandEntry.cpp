//
// Created by 赵立伟 on 2018/9/19.
//

#include <iostream>
#include "CommandEntry.h"

/**
 * 命令列表：
 *     w-表示会修改db, 有该标志下，会将命令序列追加到aof文件中
 *     A-表示是访问db的key操作，此时在访问之前会判断该键是否已过期
 **/
std::vector<CommandEntry* > flyDBCommandTable = {
       new CommandEntry("version",     versionCommand,     1, "rF",  0, NULL, 1, 1, 1, 0, 0)
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

void versionCommand(const AbstractCoordinator* coordinator,
                    std::shared_ptr<AbstractFlyClient> client) {
    if (NULL == client) {
        return;
    }

    client->addReply("FlyDB version: %s", coordinator->getFlyServer()->getVersion().c_str());
}
