//
// Created by 赵立伟 on 2018/12/1.
//

#ifndef FLYSENTINEL_FILEFIO_H
#define FLYSENTINEL_FILEFIO_H

#include "base/Fio.h"

class FileFio : public Fio {
public:
    FileFio(FILE *fp, uint64_t maxProcessingChunk);
    FileFio();
    void setFp(FILE *fp);
    void setAutosync(off_t autosync);
    int tell();
    int flush();
    FILE *getFp() const;

    class Builder {
    public:
        Builder() {
            this->fio = std::shared_ptr<FileFio>(new FileFio());
        }

        Builder& file(FILE *fp) {
            this->fio->setFp(fp);
            return *this;
        }

        Builder& autosync(off_t autosync) {
            this->fio->setAutosync(autosync);
            return *this;
        }

        Builder& maxProcessingChunk(uint64_t maxProcessingChunk) {
            this->fio->setMaxProcessingChunk(maxProcessingChunk);
            return *this;
        }

        std::shared_ptr<FileFio> build() {
            return this->fio;
        }

    private:
        std::shared_ptr<FileFio> fio = NULL;
    };

private:
    size_t baseread(void *buf, size_t len);
    size_t basewrite(const void *buf, size_t len);

    FILE *fp = NULL;
    off_t buffered = 0;                     // 距离上一次fsync所写入的字节数
    off_t autosync = 0;                     // 当写入数据>autosync时，执行fsync
};


#endif //FLYSENTINEL_FILEFIO_H
