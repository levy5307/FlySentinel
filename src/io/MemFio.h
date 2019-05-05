//
// Created by 赵立伟 on 2019/3/10.
//

#ifndef FLYSENTINEL_STRINGFIO_H
#define FLYSENTINEL_STRINGFIO_H

#include "base/Fio.h"

class MemFio : public Fio {
public:
    int tell();                         // get offset
    int flush();                        // flush data to device
    std::string getStr() const;

private:
    size_t baseread(void *buf, size_t readLen);
    size_t basewrite(const void *buf, size_t len);

    std::string str;
};


#endif //FLYSENTINEL_STRINGFIO_H
