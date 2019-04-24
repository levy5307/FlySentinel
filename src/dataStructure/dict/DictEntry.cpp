//
// Created by 赵立伟 on 2018/9/21.
//

#include "DictEntry.h"

template<class KEY, class VAL>
DictEntry<KEY, VAL>::DictEntry(const KEY key, const VAL val)
        : key(key), val(val) {}

template<class KEY, class VAL>
DictEntry<KEY, VAL>::~DictEntry() {
}

template<class KEY, class VAL>
const KEY DictEntry<KEY, VAL>::getKey() const {
    return this->key;
}

template<class KEY, class VAL>
const VAL DictEntry<KEY, VAL>::getVal() const {
    return this->val;
}

template<class KEY, class VAL>
void DictEntry<KEY, VAL>::setVal(const VAL val) {
    this->val = val;
}

template<class KEY, class VAL>
DictEntry<KEY, VAL>* DictEntry<KEY, VAL>::getNext() {
    return this->next;
};
