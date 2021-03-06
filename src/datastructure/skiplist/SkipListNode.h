//
// Created by 赵立伟 on 2018/9/24.
//

#ifndef FLYSENTINEL_SKIPLISTNODE_H
#define FLYSENTINEL_SKIPLISTNODE_H

#include <vector>
#include "SkipListDef.h"

template<class T> class SkipListNode;

template<class T>
struct SkipListLevel {
    SkipListLevel() {
        this->span = 0;
        this->next = nullptr;
    };

    uint32_t span;
    SkipListNode<T>* next;
};

template<class T>
class SkipListNode {
public:
    SkipListNode();
    SkipListNode(const T &obj, double score);
    virtual ~SkipListNode();
    const T& getObj() const;
    double getScore() const;
    SkipListNode<T> *getPrevious() const;
    void setPrevious(SkipListNode<T> *previous);
    std::vector<SkipListLevel<T> > &getLevels();
    bool scoreLtRange(SkipListRange range);
    bool scoreGtRange(SkipListRange range);
    bool scoreInRange(SkipListRange range);

private:
    T obj;
    double score;
    std::vector<SkipListLevel<T> > levels;
    SkipListNode<T> *previous;
};

#endif //FLYSENTINEL_SKIPLISTNODE_H
