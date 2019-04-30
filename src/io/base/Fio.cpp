//
// Created by 赵立伟 on 2018/12/1.
//

#include "Fio.h"

Fio::Fio(uint64_t maxProcessingChunk) {
    this->maxProcessingChunk = maxProcessingChunk;
}

Fio::Fio() {
}

Fio::~Fio() {

}

int Fio::updateChecksum(const void *buf, size_t len) {
    return 1;
}

size_t Fio::write(const void *buf, size_t len) {
    size_t totalWrite = 0;
    while (len > 0) {
        // 如果设置了读写byte上线，则单次读写不应超过其上线
        size_t writeBytes = (haveMaxProcessingChunk() && getMaxProcessingChunk() < len) ? getMaxProcessingChunk() : len;
        totalWrite += writeBytes;

        // 执行校验
        updateChecksum(buf, len);

        // 单次写入
        if (0 == this->basewrite(buf, writeBytes)) {
            return totalWrite;
        }

        buf = (char*)buf + writeBytes;
        len -= writeBytes;
        this->processedBytes += writeBytes;
    }

    return totalWrite;
}

size_t Fio::read(void *buf, size_t len) {
    size_t totalRead = 0;
    while (len > 0) {
        // 如果设置了读写byte上线，则单次读写不应超过其上线
        size_t readBytes =
                (haveMaxProcessingChunk() && getMaxProcessingChunk() < len)
                ? getMaxProcessingChunk() : len;
        totalRead += readBytes;

        // 执行校验
        updateChecksum(buf, len);

        // 单次读出
        if (0 == this->baseread(buf, readBytes)) {
            return totalRead;
        }

        buf = (char*)buf + readBytes;
        len -= readBytes;
        this->processedBytes += readBytes;
    }

    return totalRead;
}

size_t Fio::writeBulkCount(char perfix, int count) {
    std::string str = perfix + std::to_string(count) + "\r\n";
    if (this->write(str.c_str(), str.size()) <= 0) {
        return 0;
    }

    return str.size();
}

int Fio::writeBulkError(const char *err) {
    std::string str = "-ERR " + std::to_string(strlen(err)) + "\r\n";
    if (this->write(str.c_str(), str.size()) <= 0) {
        return 0;
    }

    return str.size();
}

void Fio::addReplyErrorFormat(const char *fmt, ...) {
    va_list ap;
    char msg[1024];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    int len = strlen(msg);
    // 保证msg中没有换行符, 使msg在一行内
    for (int i = 0; i < len; i++) {
        if ('\r' == msg[i] || '\n' == msg[i]) {
            msg[i] = ' ';
        }
    }

    this->writeBulkError(msg);
}

size_t Fio::writeBulkString(std::string str) {
    size_t written = 0;
    /** 头部size段 */
    if (0 == (written = this->writeBulkCount('$', str.size()))) {
        return 0;
    }

    /** 如果str不为空，写入 */
    if (str.size() > 0 && 0 == this->write(str.c_str(), str.size())) {
        return 0;
    }

    /** 尾部 */
    if (0 == this->write("\r\n", 2)) {
        return 0;
    }

    return written + str.size() + 2;
}

size_t Fio::writeBulkInt64(int64_t i) {
    std::string str = std::to_string(i);
    return this->writeBulkString(str);
}

size_t Fio::writeBulkDouble(double d) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%.17g", d);
    this->writeBulkString(buf);
}

void Fio::setMaxProcessingChunk(uint64_t maxProcessingChunk) {
    this->maxProcessingChunk = maxProcessingChunk;
}

uint64_t Fio::getMaxProcessingChunk() const {
    return this->maxProcessingChunk;
}

uint64_t Fio::getChecksum() const {
    return this->checksum;
}

void Fio::setChecksum(uint64_t checksum) {
    this->checksum = checksum;
}

size_t Fio::getProcessedBytes() const {
    return this->processedBytes;
}

bool Fio::haveMaxProcessingChunk() const {
    return 0 != this->maxProcessingChunk;
}

void Fio::setProcessedBytes(size_t processedBytes) {
    this->processedBytes = processedBytes;
}

void Fio::addProcessedBytes(size_t addBytes) {
    this->processedBytes += addBytes;
}

