//
// Created by 赵立伟 on 2018/9/19.
//

#ifndef FLYSENTINEL_COMMANDENTRY_H
#define FLYSENTINEL_COMMANDENTRY_H

#include <string>

#include "../coordinator/interface/AbstractFlyClient.h"
#include "../flyServer/interface/AbstractFlyServer.h"

typedef void (*commandProc)(const AbstractCoordinator*,
                            std::shared_ptr<AbstractFlyClient>);
typedef int (*getKeysProc)(struct CommandEntry *cmd,
                           std::shared_ptr<FlyObj> *argv,
                           int argc,
                           int *numkeys);

class CommandEntry {
public:
    CommandEntry();
    CommandEntry(commandProc proc, int flag);
    CommandEntry(const char *name,
                 commandProc proc,
                 int arity,
                 const std::string &sflags,
                 int flag,
                 getKeysProc keysProc,
                 int firstKey,
                 int lastKey,
                 int keyStep,
                 uint64_t microseconds,
                 uint64_t calls);

    const char *getName() const;
    void setName(const char *name);
    commandProc getProc() const;
    void setProc(commandProc proc);
    int getArity() const;
    void setArity(int arity);
    const std::string &getSflags() const;
    void setSflags(const std::string &sflags);
    int getFlag() const;
    void setFlag(int flag);
    void addFlag(int flag);
    bool IsWrite() const;
    void setKeysProc(getKeysProc proc);
    int getFirstKey() const;
    void setFirstKey(int firstKey);
    int getLastKey() const;
    void setLastKey(int lastKey);
    int getKeyStep() const;
    void setKeyStep(int keyStep);
    uint64_t getMicroseconds() const;
    void setMicroseconds(uint64_t microseconds);
    uint64_t getCalls() const;
    void setCalls(uint64_t calls);

public:
    class Builder {
    private:
        CommandEntry *entry = new CommandEntry();

    public:
        void name(const char *name) {
            entry->setName(name);
        }

        void proc(commandProc proc) {
            entry->setProc(proc);
        }

        void arity(int arity) {
            entry->setArity(arity);
        }

        void flag(int flag) {
            entry->setFlag(flag);
        }

        void keysProc (getKeysProc proc) {
            entry->setKeysProc(proc);
        }

        void firstKey(int fistKey) {
            entry->setFirstKey(fistKey);
        }

        void lastKey(int lastKey) {
            entry->setLastKey(lastKey);
        }

        void keyStep(int keyStep) {
            entry->setKeyStep(keyStep);
        }

        void microseconds(uint64_t microseconds) {
            entry->setMicroseconds(microseconds);
        }

        void calls(uint64_t calls) {
            entry->setCalls(calls);
        }

        CommandEntry* build() {
            return this->entry;
        }
    };

private:
    const char *name;                /** 命令名字 */
    commandProc proc;                /** 处理函数 */
    int arity;                       /** 参数数量 */
    std::string sflags;              /** 字符串形式的flag */
    int flag;
    getKeysProc keysProc;            /** 使用函数来确定所有的keys */
    int firstKey;                   /** 第一个参数是否是key */
    int lastKey;                    /** 最后一个参数是否是key */
    int keyStep;
    uint64_t microseconds, calls;
};

void versionCommand(const AbstractCoordinator*, std::shared_ptr<AbstractFlyClient>);

extern std::vector<CommandEntry*> flyDBCommandTable;

#endif //FLYSENTINEL_COMMANDENTRY_H
