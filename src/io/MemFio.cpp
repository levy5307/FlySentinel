//
// Created by 赵立伟 on 2019/3/10.
//

#include "MemFio.h"

int MemFio::tell() {
    return 0;
}

int MemFio::flush() {
    return 0;
}

std::string MemFio::getStr() const {
    return this->str;
}

size_t MemFio::baseread(void *buf, size_t readLen) {
    snprintf(reinterpret_cast<char*>(buf), readLen, "%s", this->str.c_str());
    int len = this->str.length();
    if (len > readLen) {
        this->str = this->str.substr(readLen, len);
        return readLen;
    } else {
        this->str.clear();
        return len;
    }
}

size_t MemFio::basewrite(const void *buf, size_t writeLen) {
    if (NULL == buf) {
        return 0;
    }

    const char *chars = reinterpret_cast<const char*>(buf);
    int charlen = strlen(chars);
    int realLen = charlen > writeLen ? writeLen : charlen;

    strncpy((char*)this->str.c_str(), chars, realLen);
    return realLen;
}
