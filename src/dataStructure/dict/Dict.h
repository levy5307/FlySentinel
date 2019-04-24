//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYDB_DICT_H
#define FLYDB_DICT_H

#include <map>
#include <array>
#include "HashTable.cpp"
#include "DictDef.h"
#include "../../log/interface/AbstractLogFactory.h"

template<class KEY, class VAL>
class Dict {
public:
    Dict();
    virtual ~Dict();

    int addEntry(const KEY key, const VAL val);
    int replace(const KEY key, const VAL val);
    DictEntry<KEY, VAL>* findEntry(const KEY key);
    DictEntry<KEY, VAL>* getRandomEntry();
    int fetchValue(const KEY key, VAL *val);
    int deleteEntry(const KEY key);
    bool isRehashing() const;
    void rehashSteps(uint32_t steps);
    int32_t dictScan(uint32_t cursor,
                     uint32_t steps,
                     int (*scanProc)(void* priv, KEY key, VAL val),
                     void *priv);
    int expand(uint32_t size);              // 扩容/缩容
    int shrinkToMinSize();                  // 缩容至最小容量
    int shrinkToNextPower();                // 缩容至大于used的下一个2的指数值
    int tryShrink();
    uint32_t slotNum() const;
    uint32_t size() const;
    bool setCanRehash(bool canRehash);
    bool isCanRehash() const;
    void clear(void(callback)(AbstractCoordinator*));

private:
    int32_t dictScanOneStep(uint32_t cursor,
                            int (*scanProc)(void* priv,
                                            const KEY key,
                                            const VAL val),
                            void *priv);
    uint32_t nextPower(uint32_t num);
    uint32_t revBits(uint32_t bits);
    std::array<HashTable<KEY, VAL>*, 2> ht;
    int64_t rehashIndex = -1;
    bool canRehash = true;

    AbstractLogHandler *logHandler;
};

#endif //FLYDB_DICT_H
