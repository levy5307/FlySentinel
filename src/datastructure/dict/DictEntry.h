//
// Created by 赵立伟 on 2018/9/20.
//

#ifndef FLYSENTINEL_DICTENTRY_H
#define FLYSENTINEL_DICTENTRY_H

template<class KEY, class VAL>
struct DictEntry {
 public:
    DictEntry(const KEY key, const VAL val);
    virtual ~DictEntry();
    const KEY getKey() const;
    const VAL getVal() const;
    void setVal(const VAL val);
    DictEntry<KEY, VAL>* getNext();

    KEY key;
    VAL val;
    DictEntry<KEY, VAL>* next;
};


#endif //FLYSENTINEL_DICTENTRY_H
