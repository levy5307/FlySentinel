//
// Created by 赵立伟 on 2018/12/1.
//

#include <zconf.h>
#include "FileFio.h"

FileFio::FileFio(FILE *fp, uint64_t maxProcessingChunk)
        : Fio() {
    this->fp = fp;
    this->setMaxProcessingChunk(maxProcessingChunk);
}

FileFio::FileFio() : Fio() {
}

size_t FileFio::baseread(void *buf, size_t len) {
    return fread(buf, len, 1, this->fp);
}

size_t FileFio::basewrite(const void *buf, size_t len) {
    // 写入
    int written = fwrite(buf, len, 1, this->fp);
    this->buffered += written;

    // 满足同步条件
    if (0 != this->autosync && this->buffered > this->autosync) {
        // c-lib cache --> kernel cache
        this->flush();
        // kernel cache --> disk
        fsync(fileno(this->fp));
        this->buffered = 0;
    }

    return written;
}

int FileFio::tell() {
    return ftello(this->fp);
}

int FileFio::flush() {
    return 0 == fflush(this->fp) ? 1 : 0;
}

void FileFio::setFp(FILE *fp) {
    this->fp = fp;
}

void FileFio::setAutosync(off_t autosync) {
    this->autosync = autosync;
}

FILE *FileFio::getFp() const {
    return fp;
}
