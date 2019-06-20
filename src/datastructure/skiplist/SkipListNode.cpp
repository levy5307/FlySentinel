//
// Created by 赵立伟 on 2018/9/24.
//

#include "SkipListNode.h"

template<class T>
SkipListNode<T>::SkipListNode() {

}

template<class T>
SkipListNode<T>::SkipListNode(const T &obj, double score) {
    this->obj = obj;
    this->score = score;
    this->previous = nullptr;
    this->levels.resize(SKIP_LIST_MAX_LEVEL);
}

template<class T>
std::vector<SkipListLevel<T> > &SkipListNode<T>::getLevels() {
    return levels;
}

template<class T>
bool SkipListNode<T>::scoreLtRange(SkipListRange range) {
    return range.minex ? this->score <= range.min : this->score < range.min;
}

template<class T>
bool SkipListNode<T>::scoreGtRange(SkipListRange range) {
    return range.maxex ? this->score >= range.max : this->score > range.max;
}

template<class T>
bool SkipListNode<T>::scoreInRange(SkipListRange range) {
    if (!scoreLtRange(range) && !scoreGtRange(range)) {
        return true;
    }

    return false;
}

template<class T>
const T& SkipListNode<T>::getObj() const {
    return obj;
}

template<class T>
double SkipListNode<T>::getScore() const {
    return score;
}

template<class T>
SkipListNode<T> *SkipListNode<T>::getPrevious() const {
    return previous;
}

template<class T>
void SkipListNode<T>::setPrevious(SkipListNode<T> *previous) {
    this->previous = previous;
}

template<class T>
SkipListNode<T>::~SkipListNode() {
}
