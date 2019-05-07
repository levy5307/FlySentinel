//
// Created by 赵立伟 on 2018/10/15.
//

#ifndef FLYSENTINEL_COMMANDTABLE_H
#define FLYSENTINEL_COMMANDTABLE_H

#include <string>
#include "../flyClient/FlyClient.h"
#include "../log/FileLogHandler.h"
#include "../dataStructure/dict/DictEntry.h"
#include "../commandTable/CommandEntry.h"

template<class KEY, class VAL>
class Dict;

class CommandEntry;
class CommandTable {
public:
    CommandTable(const AbstractCoordinator* coordinator);
    ~CommandTable();
    void populateCommand();
    bool dealWithCommand(std::shared_ptr<AbstractFlyClient> flyClient);

private:
    const AbstractCoordinator* coordinator;
    Dict<std::string, CommandEntry*>* commands;
};

#endif //FLYSENTINEL_COMMANDTABLE_H
