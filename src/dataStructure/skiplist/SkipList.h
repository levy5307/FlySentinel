//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYDB_SKIPLIST_H
#define FLYDB_SKIPLIST_H
#include "SkipListNode.cpp"
#include "SkipListDef.h"

// 按小-->大的顺序排序
template<class T>
class SkipList {
public:
    SkipList();
    virtual ~SkipList();
    SkipListNode<T> *getHeader() const;
    SkipListNode<T> *getTailer() const;
    uint32_t getLength() const;
    uint32_t getLevel() const;
    void insertNode(const T &obj);
    void insertNode(double score, const T &obj);
    int deleteNode(const T &obj);
    int deleteNode(double score, const T &obj);
    int deleteNode(double score, const T &obj, SkipListNode<T>** res);
    uint32_t getRank(double score, const T &obj);
    SkipListNode<T>* getNodeByRank(uint32_t rank);
    int isInRange(SkipListRange range);
    SkipListNode<T>* firstInRange(SkipListRange range);
    SkipListNode<T>* lastInRange(SkipListRange range);
    uint32_t deleteRangeByScore(SkipListRange range);
    uint32_t deleteRangeByRank(uint32_t start, uint32_t end);
    int deleteNode(SkipListNode<T>* node); // 会释放节点
    void scanAll(void (*scanProc)(void* priv, const T &obj),
                 void *priv);

private:
    uint8_t randomLevel();
    SkipListNode<T> *header, *tailer;
    uint32_t length;
    uint8_t level;
};

#endif //FLYDB_SKIPLIST_H
