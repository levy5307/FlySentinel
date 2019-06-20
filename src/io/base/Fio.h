//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYSENTINEL_FIO_H
#define FLYSENTINEL_FIO_H

#include <cstddef>
#include <cstdint>
#include <string>
#include "../../flyobj/FlyObj.h"

class Fio {
public:
    Fio(uint64_t maxProcessingChunk);
    Fio();
    virtual ~Fio();

    virtual int tell() = 0;                         // get offset
    virtual int flush() = 0;                        // flush data to device
    size_t write(const void *buf, size_t len);
    size_t read(void *buf, size_t len);
    virtual int updateChecksum(const void *buf, size_t len);

    /** higher level interace */
    size_t writeBulkCount(char perfix, int count);
    size_t writeBulkString(std::string str);
    size_t writeBulkInt64(int64_t i);
    size_t writeBulkDouble(double d);
    int writeBulkError(const char *err);
    void addReplyErrorFormat(const char *fmt, ...);

    void setMaxProcessingChunk(uint64_t maxProcessingChunk);
    uint64_t getMaxProcessingChunk() const;
    uint64_t getChecksum() const;
    void setChecksum(uint64_t checksum);
    size_t getProcessedBytes() const;
    bool haveMaxProcessingChunk() const;
    void setProcessedBytes(size_t processedBytes);
    void addProcessedBytes(size_t addBytes);

protected:
    virtual size_t baseread(void *buf, size_t len) = 0;
    virtual size_t basewrite(const void *buf, size_t len) = 0;

private:
    uint64_t checksum = 0;
    size_t processedBytes = 0;
    uint64_t maxProcessingChunk = 0;
};

#endif //FLYSENTINEL_FIO_H
