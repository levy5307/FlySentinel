//
// Created by 赵立伟 on 2018/9/20.
//

#include "Dict.h"
#include "../../utils/MiscTool.h"
#include "../../log/FileLogHandler.h"
#include "../../log/FileLogFactory.h"

extern bool canResize;

template<class KEY, class VAL>
Dict<KEY, VAL>::Dict() {
    this->ht[0] = new HashTable<KEY, VAL>(HASH_TABLE_INITIAL_SIZE);
    this->ht[1] = nullptr;
    this->rehashIndex = -1;
}

template<class KEY, class VAL>
Dict<KEY, VAL>::~Dict() {
    delete this->ht[0];
    if (isRehashing()) {
        delete this->ht[1];
    }
}

template<class KEY, class VAL>
bool Dict<KEY, VAL>::isRehashing() const {
   return this->rehashIndex >= 0;
}

template<class KEY, class VAL>
bool Dict<KEY, VAL>::isCanRehash() const {
    return this->canRehash;
}

template<class KEY, class VAL>
void Dict<KEY, VAL>::clear(void(callback)(AbstractCoordinator*)) {
    this->ht[0]->clear(callback);
    this->ht[1]->clear(callback);
    this->rehashIndex = -1;
}

template<class KEY, class VAL>
bool Dict<KEY, VAL>::setCanRehash(bool canRehash) {
    bool temp = this->canRehash;
    this->canRehash = canRehash;
    return temp;
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::addEntry(const KEY key, const VAL val) {
    /** 如果正在执行rehash, 先执行一部rehash */
    int res = 0;
    if (isRehashing()) {
        // 进行一步rehash
        rehashSteps(1);
    }

    /** 如果经过上面，rehash仍然没执行完 */
    if (isRehashing()) {
        // 插入操作
        res = this->ht[1]->addEntry(key, val);
    } else {  // 如果没在rehash, 执行插入操作；并判断是否需要扩容
        if ((res = this->ht[0]->addEntry(key, val)) > 0) {
            if (this->ht[0]->needExpand(canResize)) {
                this->ht[1] =
                        new HashTable<KEY, VAL>(this->ht[0]->getSize() * 2);
                this->rehashIndex = 0;
                rehashSteps(1);
            }
        }
    }

    // 如果插入成功，判断是否需要rehash
    return res;
}

template<class KEY, class VAL>
DictEntry<KEY, VAL>* Dict<KEY, VAL>::findEntry(const KEY key) {
    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 先查找ht[0]
    HashTable<KEY, VAL>* ht = this->ht[0];
    DictEntry<KEY, VAL>* entry = ht->findEntry(key);

    // 如果ht[0]中没有找到，并且正在进行rehash，则查找ht[1]
    if (nullptr == entry && isRehashing()) {
        ht = this->ht[1];
        entry = ht->findEntry(key);
    }

    return entry;
}

template<class KEY, class VAL>
DictEntry<KEY, VAL>* Dict<KEY, VAL>::getRandomEntry() {
    if (0 == this->size()) {
        return nullptr;
    }

    /** 随机获取某个DictEntry列表(每个index对应一个DictEntry列表) */
    DictEntry<KEY, VAL>* dictEntry = nullptr;
    uint32_t index = 0;
    if (this->isRehashing()) {
        // 先进行一步rehash
        rehashSteps(1);
        do {
            index = this->rehashIndex + (random() % (this->ht[0]->getSize()
                    + this->ht[1]->getSize() - this->rehashIndex));
            dictEntry = (index > this->ht[0]->getSize())
                    ? this->ht[1]->getEntryBy(index - this->ht[0]->getSize())
                    : this->ht[0]->getEntryBy(index);
        } while (nullptr == dictEntry);
    } else {
        do {
            index = this->ht[0]->getIndex(random());
            dictEntry = this->ht[0]->getEntryBy(index);
        } while (nullptr == dictEntry);
    }

    int length = 0;
    DictEntry<KEY, VAL> *realEntry = dictEntry;
    while (dictEntry != nullptr) {
        length++;
        dictEntry = dictEntry->getNext();
    }

    /** 随机获取该链表中的某个entry */
    dictEntry = realEntry;
    int num = random() % length;
    while (num-- > 0) {
        dictEntry = dictEntry->getNext();
    }

    return dictEntry;
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::fetchValue(const KEY key, VAL *val) {
    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    /** 查询key, 获取不到返回-1 */
    DictEntry<KEY, VAL>* entry = findEntry(key);
    if (nullptr == entry) {
        return -1;
    }

    *val = entry->val;
    return 1;
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::deleteEntry(const KEY key) {
    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 先从ht[0]中尝试删除, 如果删除成功
    if ((this->ht[0]->deleteEntry(key)) > 0) {
        // 没有处于rehash过程中 && 需要缩容，则进行缩容
        if (!isRehashing() && this->ht[0]->needShrink(canResize)) {
            this->ht[1] = new HashTable<KEY, VAL>(this->ht[0]->getSize() / 2);
            this->rehashIndex = 0;
            rehashSteps(1);
        }
        return 1;
    } else if (isRehashing()) {
        /**
         * 如果ht[0]中删除失败，并且正在rehash过程中,
         * 则需要从ht[1]中进行查找删除
         */
        return this->ht[1]->deleteEntry(key);
    }

    return -1;
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::replace(const KEY key, const VAL val) {
    // 先进行一步rehash
    if (isRehashing()) {
        rehashSteps(1);
    }

    // 先执行插入步骤，如果插入成功，说明之前没有该key，直接返回成功
    if (addEntry(key, val) > 0) {
        return 1;
    }

    // 如果插入失败，说明之前存在该key，需要替换value
    DictEntry<KEY, VAL>* entry = findEntry(key);
    entry->val = val;
    return 1;
}

template<class KEY, class VAL>
void Dict<KEY, VAL>::rehashSteps(uint32_t steps) {
    /** 如果不允许rehash, 直接返回 */
    if (!canRehash) {
        return;
    }

    for (uint32_t i = 0; i < steps && !this->ht[0]->isEmpty(); i++) {
        // 找到存在元素的index
        DictEntry<KEY, VAL>* entry = nullptr;
        while (nullptr == entry && this->rehashIndex < this->ht[0]->getSize()) {
            entry = this->ht[0]->getEntryBy(this->rehashIndex++);
        }

        // 对该index下的链表中所有元素进行迁移
        while (entry != nullptr) {
            this->ht[1]->addEntry(entry->getKey(), entry->getVal());
            this->ht[0]->deleteEntry(entry->getKey());
            entry = entry->next;
        }
    }

    // 已经rehash完毕
    if (this->ht[0]->isEmpty()) {
        delete this->ht[0];
        this->ht[0] = this->ht[1];
        this->ht[1] = nullptr;
        this->rehashIndex = -1;
    }

    return;
}

template<class KEY, class VAL>
int32_t Dict<KEY, VAL>::dictScan(uint32_t cursor,
                        uint32_t steps,
                        int (*scanProc)(void* priv, KEY key, VAL val),
                        void *priv) {
    uint32_t nextCursor = cursor;
    for (uint32_t i = 0; i < steps; i++) {
        nextCursor = dictScanOneStep(nextCursor, scanProc, priv);
        /** nextCursor = 0代表scan完毕， nextCursor=-1代表遍历过程失败 */
        if (nextCursor <= 0) {
            return nextCursor;
        }
    }

    return nextCursor;
}

template<class KEY, class VAL>
int32_t Dict<KEY, VAL>::dictScanOneStep(
        uint32_t cursor,
        int (*scanProc)(void* priv, const KEY key, const VAL val),
        void *priv) {
    HashTable<KEY, VAL>* ht0 = this->ht[0];
    HashTable<KEY, VAL>* ht1 = this->ht[1];
    if (isRehashing()) {
        // 确保ht0保存小的hashtable
        if (ht0->getSize() > ht1->getSize()) {
            HashTable<KEY, VAL>* tmp = ht0;
            ht0 = ht1;
            ht1 = tmp;
        }

        // scan ht0
        uint32_t index = ht0->getIndex(cursor);
        if (-1 == ht0->scanEntries(index, scanProc, priv)) {
            return -1;
        }

        /**
         * scan ht1: 由于ht1 > ht0, 所以ht1的大小是ht0的二倍
         * 1.以ht0为基准去进行迭代遍历，函数最后进位的时候也是以ht0的坐标去递进(使用了ht0的掩码),
         *   对于ht1中的遍历，只是查找ht1中与ht0中相对应的entry, 以防止遗漏
         * 2.ht1中有两个连续的entry与ht0中的entry相对应, 因此需要scan ht1中两个entry,
         *   例如ht0->size = 2, ht1->size = 4。那么ht0->mask = 0001, ht1->mask = 0011,
         *   此时ht0中与index=0001相对应的ht1中的元素是0001和0011
         * 3.ht1中的第二个scan就是将index的第二位置1, 以遍历相应的第二个桶
         */
        index = ht1->getIndex(cursor);
        if (-1 == ht1->scanEntries(index, scanProc, priv)) {
            return -1;
        }
        index |= (~ht0->getMask() & ht1->getMask())
                | (ht0->getMask() & ~ht1->getMask());
        if (-1 == ht1->scanEntries(index, scanProc, priv)) {
            return -1;
        }
    } else {
        // 如果未处于rehash, 只访问ht[0]就可以了
        uint32_t index = ht0->getIndex(cursor);
        if (-1 == ht0->scanEntries(index, scanProc, priv)) {
            return -1;
        }
    }

    cursor |= ~ht0->getMask();
    // 高位加1，向低位进位
    cursor = revBits(cursor);
    cursor++;
    cursor = revBits(cursor);
    return cursor;
}

// reserve bit位， 例如：b1 b2 b3 b4，经过reserve后变成b4 b3 b2 b1
template<class KEY, class VAL>
uint32_t Dict<KEY, VAL>::revBits(uint32_t bits) {
    // bit size; must be power of 2
    uint32_t s = 8 * sizeof(bits);
    uint32_t mask = ~0;
    while ((s >>= 1) > 0) {
        mask ^= (mask << s);
        bits = ((bits >> s) & mask) | ((bits << s) & ~mask);
    }
    return bits;
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::shrinkToMinSize() {
    /** 如果正处于rehash过程中，或者ht[0]不允许resize, 返回-1 */
    if (this->isRehashing() || !canResize) {
        return -1;
    }

    int minimal = this->ht[0]->getUsed();
    if (minimal < HASH_TABLE_INITIAL_SIZE) {
        minimal = HASH_TABLE_INITIAL_SIZE;
    }

    this->expand(minimal);
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::shrinkToNextPower() {
    /** 如果正处于rehash过程中，或者ht[0]不允许resize, 返回-1 */
    if (this->isRehashing() || !canResize) {
        return -1;
    }

    int minimal = this->ht[0]->getUsed();
    if (minimal < HASH_TABLE_INITIAL_SIZE) {
        minimal = HASH_TABLE_INITIAL_SIZE;
    } else {
        /** 获取minimal的next power */
        minimal = nextPower(minimal);
    }

    this->expand(minimal);
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::tryShrink() {
    /** 如果正处于rehash过程中，或者ht[0]不允许缩容, 返回-1 */
    if (this->isRehashing() || !this->ht[0]->needShrink(canResize)) {
        return -1;
    }

    this->shrinkToNextPower();
}

template<class KEY, class VAL>
int Dict<KEY, VAL>::expand(uint32_t size) {
    uint32_t expandSize = nextPower(size);
    // 如果正在rehash, 或者扩容大小 < 目前已使用空间
    if (isRehashing() || expandSize < this->ht[0]->getUsed()) {
        return -1;
    }

    // 容量没变化
    if (expandSize == this->ht[0]->getSize()) {
        return -1;
    }

    // 扩(缩)容，并执行rehash
    this->ht[1] = new HashTable<KEY, VAL>(expandSize);
    this->rehashIndex = 0;
    rehashSteps(1);

    return 1;
}

template<class KEY, class VAL>
uint32_t Dict<KEY, VAL>::slotNum() const {
    uint32_t size = this->ht[0]->getSize();
    if (nullptr != this->ht[1]) {
        size += this->ht[1]->getSize();
    }

    return size;
}

template<class KEY, class VAL>
uint32_t Dict<KEY, VAL>::size() const {
    uint32_t size = this->ht[0]->getUsed();
    if (nullptr != this->ht[1]) {
        size += this->ht[1]->getUsed();
    }

    return size;
}

template<class KEY, class VAL>
uint32_t Dict<KEY, VAL>::nextPower(uint32_t num) {
    int i = HASH_TABLE_INITIAL_SIZE;

    // 如果num大于32位数字中最大的power，则返回maxPower
    uint32_t maxPower = 1 << 31;
    if (num >= maxPower) {
        return maxPower;
    }

    while (1) {
        if (i >= num) {
            break;
        } else {
            i *= 2;
        }
    }

    return i;
}
