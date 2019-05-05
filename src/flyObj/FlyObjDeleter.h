//
// Created by 赵立伟 on 2019/1/27.
//

#ifndef FLYSENTINEL_FLYOBJDELETER_H
#define FLYSENTINEL_FLYOBJDELETER_H

template<class T>
struct MyDeleter {
    inline void operator()(void* pData) {
        delete static_cast<T*>(pData);
    }
};

template<class T>
struct MyDeleter<T[]> {
    inline void operator()(void* pData) {
        delete[] static_cast<T*>(pData);
    }
};

#endif //FLYSENTINEL_FLYOBJDELETER_H
