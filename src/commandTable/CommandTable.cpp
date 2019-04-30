//
// Created by 赵立伟 on 2018/10/15.
//

#include <iostream>
#include "CommandTable.h"
#include "CommandEntry.h"
#include "../utils/MiscTool.h"
#include "../log/FileLogFactory.h"
#include "../dataStructure/dict/Dict.cpp"
#include "CommandTableDef.h"

CommandTable::CommandTable(const AbstractCoordinator* coordinator) {
    this->commands = new Dict<std::string, CommandEntry*>();
    this->logHandler = logFactory->getLogger();
    this->coordinator = coordinator;
    this->populateCommand();
}

CommandTable::~CommandTable() {
    delete this->commands;
}

void CommandTable::populateCommand() {
    int num = flyDBCommandTable.size();
    for (int i = 0; i < num; i++) {
        CommandEntry *entry = flyDBCommandTable[i];
        const char *f = entry->getSflags().c_str();
        while (*f != '\0') {
            switch (*f) {
                case 'w':
                    entry->addFlag(CMD_WRITE);
                    break;
                case 'r':
                    entry->addFlag(CMD_READONLY);
                    break;
                case 'm':
                    entry->addFlag(CMD_DENYOOM);
                    break;
                case 'a':
                    entry->addFlag(CMD_ADMIN);
                    break;
                case 'p':
                    entry->addFlag(CMD_PUBSUB);
                    break;
                case 's':
                    entry->addFlag(CMD_NOSCRIPT);
                    break;
                case 'R':
                    entry->addFlag(CMD_RANDOM);
                    break;
                case 'S':
                    entry->addFlag(CMD_SORT_FOR_SCRIPT);
                    break;
                case 'l':
                    entry->addFlag(CMD_LOADING);
                    break;
                case 't':
                    entry->addFlag(CMD_STALE);
                    break;
                case 'M':
                    entry->addFlag(CMD_SKIP_MONITOR); break;
                case 'k':
                    entry->addFlag(CMD_ASKING);
                    break;
                case 'F':
                    entry->addFlag(CMD_FAST);
                    break;
                default:
                    logHandler->logWarning("Unrecognized flag type! %c", *f);
                    exit(1);
            }
            f++;
        }
        this->commands->addEntry(entry->getName(), entry);
    }
}

bool CommandTable::dealWithCommand(std::shared_ptr<AbstractFlyClient> flyClient) {
    std::string *command = reinterpret_cast<std::string*>(flyClient->getArgv()[0]->getPtr());
    DictEntry<std::string, CommandEntry*>* dictEntry = this->commands->findEntry(*command);
    if (NULL == dictEntry) {
        flyClient->addReply("wrong command type: %s", command);
        return -1;
    }

    /** 如果参数数量少，返回失败 */
    int arity = dictEntry->getVal()->getArity();
    if (flyClient->getArgc() < arity) {
        flyClient->addReply("too few parameters: %d, need: %d ",
                            flyClient->getArgc(), arity);
        return -1;
    }

    time_t now = getCurrentTime();
    /** 处理命令 */
    dictEntry->getVal()->getProc()(this->coordinator, flyClient);

    time_t now2 = getCurrentTime();
    time_t use_time = now2 - now;
    coordinator->getFlyServer()->PushSlowlogIfNeed(flyClient, now, use_time);

    /** 添加命令序列到相应的缓冲中 */
    if (dictEntry->getVal()->IsWrite()) {
        coordinator->getAofHandler()->feedAppendOnlyFile(
                flyClient->getFlyDB()->getId(),
                flyClient->getArgv(),
                flyClient->getArgc());
        return true;
    }

    return false;
}

