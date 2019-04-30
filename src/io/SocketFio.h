//
// Created by levy on 2019/3/29.
//

#ifndef FLYDB_SOCKETFIO_H
#define FLYDB_SOCKETFIO_H

#include <vector>
#include "base/Fio.h"

class SocketFio : public Fio {
public:
    SocketFio();

    /** get offset */
    int tell();

    /**
     * flush data to fds
     *  return: 0-failure; 1-success
     * */
    int flush();

    int getState(int idx);

    class Builder {
    public:
        Builder() {
            this->fio = std::shared_ptr<SocketFio>(new SocketFio());
        }

        Builder& maxProcessingChunk(uint64_t maxProcessingChunk) {
            this->fio->setMaxProcessingChunk(maxProcessingChunk);
            return *this;
        }

        Builder& fds(std::vector<int> &fds) {
            this->fio->setFds(fds);
            this->fio->initStates(fds.size());
            return *this;
        }

        std::shared_ptr<SocketFio> build() {
            return this->fio;
        }

    private:
        std::shared_ptr<SocketFio> fio = NULL;
    };

private:
    size_t baseread(void *buf, size_t readLen);
    /** basewrite只是将数据存入缓冲里，flush才是真正的写入socket */
    size_t basewrite(const void *buf, size_t len);
    void setFds(const std::vector<int> &fds);
    void initStates(int size);

private:
    std::vector<int> fds;
    std::vector<int> states;
    off_t pos;
    std::string buf;
};


#endif //FLYDB_SOCKETFIO_H
