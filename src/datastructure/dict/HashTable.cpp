//
// Created by 赵立伟 on 2018/9/20.
//
#include <iostream>
#include <functional>
#include "HashTable.h"

template<class KEY, class VAL>
HashTable<KEY, VAL>::HashTable(uint32_t size) : size(size) {
    this->table = new DictEntry<KEY, VAL>*[size];
    for (uint32_t i = 0; i < size; i++) {
        this->table[i] = nullptr;
    }
    this->used = 0;
    this->mask = size - 1;
}

template<class KEY, class VAL>
HashTable<KEY, VAL>::~HashTable() {
    delete this->table;
}

template<class KEY, class VAL>
uint32_t HashTable<KEY, VAL>::getIndexWithKey(const KEY key) const {
    return std::hash<KEY>()(key) & this->mask;
}

template<class KEY, class VAL>
uint32_t HashTable<KEY, VAL>::getIndex(uint32_t cursor) const {
    return cursor & this->mask;
}

template<class KEY, class VAL>
int HashTable<KEY, VAL>::scanEntries(
        uint32_t index,
        int (*scanProc)(void* priv, const KEY key, const VAL val),
        void* priv) {
    DictEntry<KEY, VAL>* entry = this->getEntryBy(index);
    while (nullptr != entry) {
        if (-1 == scanProc(priv, entry->getKey(), entry->getVal())) {
            return -1;
        }
        entry = entry->next;
    }
    return 0;
}

template<class KEY, class VAL>
int HashTable<KEY, VAL>::addEntry(const KEY key, const VAL val) {
    uint32_t index = getIndexWithKey(key);

    // 判断是否已经有相同的键，如果有，则不能继续插入
    if (hasKey(key)) {
        return -1;
    }

    // 将该entry插入头部
    DictEntry<KEY, VAL>* entry = new DictEntry<KEY, VAL>(key, val);
    DictEntry<KEY, VAL>* head = this->table[index];
    this->table[index] = entry;
    entry->next = head;

    this->used++;
    return 1;
}

template<class KEY, class VAL>
DictEntry<KEY, VAL>* HashTable<KEY, VAL>::findEntry(const KEY key) {
    uint32_t index = getIndexWithKey(key);
    DictEntry<KEY, VAL>* node = this->table[index];
    while (node != nullptr) {
        if (node->key == key) {
           return node;
        }
        node = node->next;
    }
    return nullptr;
}

template<class KEY, class VAL>
int HashTable<KEY, VAL>::deleteEntry(const KEY key) {
    uint32_t index = getIndexWithKey(key);
    DictEntry<KEY, VAL>* node = this->table[index];
    if (node != nullptr) {
        // 如果要删除的key是头结点
        if (node->key == key) {
            this->table[index] = node->next;
            delete node;
            this->used--;
            return 1;
        }

        // 如果不是头结点，则查找链表中是否有该节点
        while (node->next != nullptr) {
            if (node->next->key == key) {
                DictEntry<KEY, VAL>* tmp = node->next;
                node->next = node->next->next;
                delete tmp;
                this->used--;
                return 1;
            }
            node = node->next;
        }
    }

    return -1;
}

template<class KEY, class VAL>
bool HashTable<KEY, VAL>::hasKey(const KEY key) {
    return this->findEntry(key) != nullptr;
}

template<class KEY, class VAL>
bool HashTable<KEY, VAL>::needExpand(bool canResize) const {
    /** 如果允许resize， 则used > size时就需要扩容*/
    if (canResize && this->used > this->size) {
        return true;
    }

    /**
     * 如果不允许resize, 则需要used > size * NEED_FORCE_REHASH_RATIO才允许扩容,
     * 此时一般是由于aof或者fdb child thread正在进行持久化，减小内存压力
     **/
    if (!canResize && this->used > this->size * NEED_FORCE_REHASH_RATIO) {
        return true;
    }

    return false;
}

template<class KEY, class VAL>
bool HashTable<KEY, VAL>::needShrink(bool canResize) const {
    return canResize && (this->size > HASH_TABLE_INITIAL_SIZE)
           && (this->used*100/this->size < HASHTABLE_MIN_FILL);
}

template<class KEY, class VAL>
uint32_t HashTable<KEY, VAL>::getSize() const {
    return this->size;
}

template<class KEY, class VAL>
uint32_t HashTable<KEY, VAL>::getUsed() const {
    return this->used;
}

template<class KEY, class VAL>
bool HashTable<KEY, VAL>::isEmpty() const {
    return 0 == this->used;
}

template<class KEY, class VAL>
DictEntry<KEY, VAL>* HashTable<KEY, VAL>::getEntryBy(uint32_t index) const {
    return this->table[index];
}

template<class KEY, class VAL>
uint32_t HashTable<KEY, VAL>::getMask() const {
    return this->mask;
}

template<class KEY, class VAL>
void HashTable<KEY, VAL>::clear(void(callback)(AbstractCoordinator *)) {
    for (int i = 0; i < this->size; i++) {
        if (this->used <= 0) {
            return;
        }

        if (nullptr != callback && 0 == i % 65535) {
            extern AbstractCoordinator *coordinator;
            callback(coordinator);
        }

        DictEntry<KEY, VAL> *dictEntry = this->table[i];
        while (dictEntry != nullptr) {
            DictEntry<KEY, VAL> *next = dictEntry->getNext();
            delete dictEntry;
            dictEntry = next;
            this->used--;
        }
    }
};

