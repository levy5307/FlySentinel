//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipList.h"
#include "SkipListDef.h"

template<class T>
SkipList<T>::SkipList() {
    this->level = 1;
    this->length = 0;
    this->header = this->tailer = new SkipListNode<T>(NULL, 0);
}

template<class T>
SkipList<T>::~SkipList() {
    SkipListNode<T>* prev = this->header;
    SkipListNode<T>* node = prev->getLevels()[0].next;
    while (nullptr != node) {
        delete node;
        node = prev->getLevels()[0].next;
    }

    delete this->header;
}

template<class T>
SkipListNode<T> *SkipList<T>::getHeader() const {
    return header;
}

template<class T>
SkipListNode<T> *SkipList<T>::getTailer() const {
    return tailer;
}

template<class T>
uint32_t SkipList<T>::getLength() const {
    return length;
}

template<class T>
uint32_t SkipList<T>::getLevel() const {
    return level;
}

template<class T>
void SkipList<T>::insertNode(const T &obj) {
    this->insertNode(0, obj);
}

template<class T>
void SkipList<T>::insertNode(double score, const T &obj) {
    // 随机获取level
    uint8_t randLevel = randomLevel();
    uint8_t finalLevel = randLevel > this->level ? randLevel : this->level;

    std::vector<SkipListNode<T>*> forwards(finalLevel);
    std::vector<uint32_t> rank(finalLevel + 1, 0); // 第level+1个作为哨兵
    /*
     * **是待插入点node， 插入前节点是prevNode[level]
     * | ============================================> |    level = 3
     * | =================================**=> | ====> |    level = 2
     * | ====================> | =========**=> | ====> |    level = 1
     * | ====> | ====> | ====> | ====> | =**=> | ====> |    level = 0
     * a0                      a1      a2      a3
     * 插入前: rank[0] = a2, rank[1] = a1, rank[2] = a0
     * 插入后: prevNode[1].span = a2 - a1 + 1 = rank[0] - rank[1] + 1
     *        prevNode[2].span = a2 - a0 + 1 = rank[0] - rank[2] + 1
     *        ====> peveNode[i].span = rank[0] - rank[i] + 1
     *        ====> node[level] = 原prevNode[level].span - prevNode[level].span + 1
     */
    // forwards代表待插入点的前一个节点, rank则代表该节点在跳跃表中的排序
    SkipListNode<T>* temp = this->header;
    for (int i = finalLevel - 1; i >= 0; i--) {
        rank[i] = rank[i+1];                  // 哨兵用于这里
        SkipListNode<T>* next = temp->getLevels()[i].next;
        while (nullptr != next && (next->getScore() < score
                   || (next->getScore() == score
                       && next->getObj() < obj))) {
            rank[i] += temp->getLevels()[i].span;
            temp = next;
            next = next->getLevels()[i].next;
        }
        forwards[i] = temp;
    }

    // 假如level > 跳跃表当前level, 初始化新增加的几个层，用于后面计算
    for (uint32_t i = this->level; i < randLevel; i++) {
        forwards[i] = header;
        rank[i] = 0;
        header->getLevels()[i].span = rank[0];
    }

    // 设置tailer以及previous指针
    SkipListNode<T> *nodeToInsert = new SkipListNode<T>(obj, score);
    nodeToInsert->setPrevious(forwards[0] == header ? nullptr : forwards[0]);
    if (nullptr == forwards[0]->getLevels()[0].next) {
        this->tailer = nodeToInsert;
    } else {
        forwards[0]->getLevels()[0].next->setPrevious(nodeToInsert);
    }

    // 实际的插入操作，并计算相应节点的span
    for (uint32_t i = 0; i < randLevel; i++) {
        nodeToInsert->getLevels()[i].next = forwards[i]->getLevels()[i].next;
        forwards[i]->getLevels()[i].next = nodeToInsert;
        nodeToInsert->getLevels()[i].span = forwards[i]->getLevels()[i].span - (rank[0] - rank[i]);
        forwards[i]->getLevels()[i].span = rank[0] - rank[i] + 1;
    }

    // 如果level小于跳跃表的level，将没有进行插入的level的前节点span+1
    for (uint32_t i = randLevel; i < this->level; i++) {
        if (forwards[i]->getLevels()[i].next != nullptr) {
            forwards[i]->getLevels()[i].span += 1;
        }
    }

    this->level = finalLevel;
    this->length++;
}

/**
 * 以SKIP_LIST_MAX_LEVEL = 4为例，maxNum = 8
 * rand:    1      2       3       4       5       6       7       8
 * 概率：   1/4    1/4     |-> 1/2 <-|     |--------> 1/2 <----------|
 * level:   4      3           2                      1
 */
template<class T>
uint8_t SkipList<T>::randomLevel() {
    uint32_t maxNum = 1 << (SKIP_LIST_MAX_LEVEL - 1);

    srand((unsigned)time(nullptr));
    uint32_t randNum = ((rand() & 0xFFFF) << 16 | (rand() & 0xFFFF)) % maxNum;

    // 根据随机数获取具体的level
    uint8_t power = 0;
    for (uint32_t num = maxNum; num >= 1; num = num >> 1) {
        if (randNum > num) {
            break;
        }
        power++;
    }
    return power;
}

template<class T>
int SkipList<T>::deleteNode(const T &obj) {
    SkipListNode<T> *node = new SkipListNode<T>();
    return this->deleteNode(0, obj);
}

template<class T>
int SkipList<T>::deleteNode(double score, const T &obj) {
    SkipListNode<T> *node = new SkipListNode<T>();
    return this->deleteNode(score, obj, &node);
}

template<class T>
int SkipList<T>::deleteNode(double score, const T &obj, SkipListNode<T>** res) {
    // forward找到待查节点的前节点
    std::vector<SkipListNode<T>*> forwards(this->level);
    for (int l = this->level - 1; l >= 0; l--) {
        SkipListNode<T>* prevNode = this->header;
        SkipListNode<T>* node = prevNode->getLevels()[l].next;
        // 找到node==nullptr或者score、obj都相等的node
        while (node != nullptr
               && (node->getScore() < score
                   || node->getObj() < obj)) {
            prevNode = node;
            node = prevNode->getLevels()[l].next;
        }
        forwards[l] = prevNode;
    }

    // 没有找到符合条件的node
    if (nullptr == forwards[0]->getLevels()[0].next
        || (forwards[0]->getLevels()[0].next->getScore() != score
            && forwards[0]->getLevels()[0].next->getObj() < obj)) {
        return -1;
    } else {  // 处理previous指针
        SkipListNode<T>* node = *res = forwards[0]->getLevels()[0].next;
        if (nullptr == node->getLevels()[0].next) {
            this->tailer = forwards[0];
        } else {
            node->getLevels()[0].next->setPrevious(forwards[0]);
        }
    }

    // 对于level >= 0的所有层，都要进行span更改
    uint8_t resvLevel = this->level;
    for (int i = 0; i < resvLevel; i++) {
        SkipListNode<T>* node = forwards[i]->getLevels()[i].next;
        // 如果从该层开始找不到待删除节点并且forward的下一个节点是nullptr，上面的那些层也不必找了
        if (nullptr == node) {
            break;
        } else {
            // 该层只剩header和待删除节点，那么level-1
            if (forwards[i] == this->header && nullptr == node->getLevels()[i].next) {
                this->level--;
            }

            forwards[i]->getLevels()[i].next = node->getLevels()[i].next;
            forwards[i]->getLevels()[i].span += node->getLevels()[i].span - 1;
        }
    }

    this->length--;
    return 0;
}

template<class T>
uint32_t SkipList<T>::getRank(double score, const T &obj) {
    int totalSpan = 0;

    SkipListNode<T>* prev = this->header;
    for (int l = this->level - 1; l >= 0; l--) {
        SkipListNode<T>* next = prev->getLevels()[l].next;

        /**
          * 如果next小的话，往后遍历。该循环停下来有三个条件：
          *      1.next==nullptr
          *      2.找到合适的node
          *      3.当前node超过了查找的值，此时需要向下一层寻找
         **/
        while (next != nullptr
               && (next->getScore() < score
                   || *(next->getObj()) < *obj)) {
            prev = next;
            next = prev->getLevels()[l].next;
            totalSpan += prev->getLevels()[l].span;
        }

        // 如果找到了，直接返回
        if (next != nullptr
            && next->getScore() == score
            && *(next->getObj()) < *obj) {
            totalSpan += prev->getLevels()[l].span;
            return totalSpan;
        }
    }

    return -1;
}

template<class T>
SkipListNode<T>* SkipList<T>::getNodeByRank(uint32_t rank) {
    SkipListNode<T>* node = this->header;

    for (int l = this->level - 1; l >= 0; l--) {
        while (node->getLevels()[l].next != nullptr
               && node->getLevels()[l].span < rank) {
            rank -= node->getLevels()[l].span;
            node = node->getLevels()[l].next;
        }

        if (node->getLevels()[l].span == rank) {
            return node->getLevels()[l].next;
        }
    }

    return nullptr;
}

template<class T>
int SkipList<T>::isInRange(SkipListRange range) {
    // range入参有问题
    if (range.max < range.min
        || (range.max == range.min && (range.maxex || range.minex))) {
        return -1;
    }

    // 跳跃表中无节点
    if (this->tailer == this->header) {
        return 0;
    }

    // 如果tail.score <= range并且表中第一个元素 >= range
    SkipListNode<T>* node = header->getLevels()[0].next;
    if (!this->tailer->scoreGtRange(range)
        && !node->scoreLtRange(range)) {
        return 1;
    }

    return 0;
}

template<class T>
SkipListNode<T>* SkipList<T>::firstInRange(SkipListRange range) {
    SkipListNode<T>* prev = this->header;
    SkipListNode<T>* node;
    for (int l = this->level - 1; l >= 0; l--) {
        node = prev->getLevels()[l].next;
        while (node != nullptr && node->scoreLtRange(range)) {
            prev = node;
            node = prev->getLevels()[l].next;
        }

        // 找到第一个属于范围内的节点，并且prev对应的span等于1
        if (node != nullptr
            && node->scoreInRange(range)
            && 1 == prev->getLevels()[l].span) {
            return node;
        }
    }

    return nullptr;
}

template<class T>
SkipListNode<T>* SkipList<T>::lastInRange(SkipListRange range) {
    SkipListNode<T>* prev = this->header;
    SkipListNode<T>* node;
    for (int l = this->level - 1; l >= 0; l--) {
        node = prev->getLevels()[l].next;
        // 找到该层score > range的节点，或者是找到链表的最后一个节点
        while (node != nullptr && !node->scoreGtRange(range)) {
            prev = node;
            node = prev->getLevels()[l].next;
        }

        /**
         * 找到了node->score > range，并且要符合以下两点：
         * 1.prev到node的跨度只有1或者prev是链表最后一个节点
         * 2.prev->score在range范围内
         */
        if ((1 == prev->getLevels()[l].span || node == nullptr)
            && prev->scoreInRange(range)) {
            return prev;
        }
    }

    return nullptr;
}

template<class T>
uint32_t SkipList<T>::deleteRangeByScore(SkipListRange range) {
    SkipListNode<T>* node = firstInRange(range);
    if (node == nullptr) {
        return 0;
    }

    // 获取前一个节点
    SkipListNode<T>* prev = node->getPrevious();
    if (nullptr == prev) {
        prev = this->header;
    }

    int removed = 0;
    while (node != nullptr && node->scoreInRange(range)) {
        deleteNode(node);
        this->length--;
        removed++;
        node = prev->getLevels()[0].next;
    }

    return removed;
}

template<class T>
uint32_t SkipList<T>::deleteRangeByRank(uint32_t start, uint32_t end) {
    if (end < start) {
        return 0;
    }

    uint32_t totalSpan = 0;
    uint32_t removed = 0;

    // 找到第一个在[start, end]范围内的node
    SkipListNode<T>* node = this->header;
    for (int l = this->level - 1; l >= 0; l--) {
        while (node->getLevels()[l].next != nullptr
               && totalSpan + node->getLevels()[l].span < start) {
            totalSpan += node->getLevels()[l].span;
            node = node->getLevels()[l].next;
        }

        if (1 == node->getLevels()[l].span
            && totalSpan + node->getLevels()[l].span >= start) {
            break;
        }
    }

    totalSpan += node->getLevels()[0].span;
    if (totalSpan < start) {
        return 0;
    }

    // 依次删除所有范围内节点
    SkipListNode<T>* prev = node;
    node = prev->getLevels()[0].next;
    while (node != nullptr && totalSpan <= end) {
        deleteNode(node);
        this->length--;
        removed++;
        node = prev->getLevels()[0].next;
        totalSpan += node->getLevels()[0].span;
    }

    return removed;
}

template<class T>
void SkipList<T>::scanAll(void (*scanProc)(void* priv, const T &obj),
                          void *priv) {
    SkipListNode<T> *node = this->tailer;
    while (node != this->header) {
        scanProc(priv, node->getObj());
        node = node->getPrevious();
    }
}

template<class T>
int SkipList<T>::deleteNode(SkipListNode<T>* node) {
    if (nullptr == node) {
        return -1;
    }

    // 对每一层做摘除操作
    int res = 0;
    SkipListNode<T>* prev = node->getPrevious();
    if (nullptr == prev) {
        prev = this->header;
    }
    uint8_t resvLevel = this->level;
    for (int l = 0; l < resvLevel; l++) {
        if (prev->getLevels()[l].next == node) {
            prev->getLevels()[l].span += node->getLevels()[l].span - 1;
            prev->getLevels()[l].next = node->getLevels()[l].next;
            if (prev == this->header && nullptr == node->getLevels()[l].next) {
                this->level--;
            }
            res = 1;
        } else {
            // 如果从该层开始找不到待删除节点并且prev的下一个节点是nullptr，上面的那些层也不必找了
            if (prev->getLevels()[l].next == nullptr) {
                break;
            } else {
                prev->getLevels()[l].span--;
            }
        }
    }

    // 设置previous及tailer
    if (nullptr == node->getLevels()[0].next) {
        this->tailer = prev;
    } else {
        node->getLevels()[0].next->setPrevious(prev);
    }

    // 释放该节点
    delete node;

    return res;
}

