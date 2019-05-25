//
// Created by levy on 2019/3/17.
//

#include "SharedObjects.h"

SharedObjects::SharedObjects(AbstractCoordinator *coordinator) {
    this->coordinator = coordinator;

    this->mbulkHeaders.resize(OBJ_SHARED_BULKHDR_LEN);
    for (int i = 0; i < OBJ_SHARED_BULKHDR_LEN; i++) {
        char buf[10];
        snprintf(buf, sizeof(buf), "*%d\r\n", i);
        this->mbulkHeaders.push_back(this->coordinator->getFlyObjStringFactory()->getObject(new std::string(buf)));
    }

    this->pong = this->coordinator->getFlyObjStringFactory()
                ->getObject(new std::string("+pong\r\n"));

    this->ping = this->coordinator->getFlyObjStringFactory()
            ->getObject(new std::string("ping"));

    this->ok = this->coordinator->getFlyObjStringFactory()
            ->getObject(new std::string("+ok\r\n"));

    for (int i = 0; i < PROTO_SHARED_SELECT_CMDS; i++) {
        char buf[50];
        std::string dbidStr = std::to_string(i);
        snprintf(buf, sizeof(buf), "*2\r\n$6\r\nSELECT\r\n$%d\r\n%s\r\n", dbidStr.length(), dbidStr.c_str());
        this->select.push_back(this->coordinator->getFlyObjStringFactory()->getObject(new std::string(buf)));
    }

    /** pub/sub share object */
    this->messagebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$7\r\nmessage\r\n"));
    this->pmessagebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$8\r\npmessage\r\n"));
    this->subscribebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$9\r\nsubscribe\r\n"));
    this->unsubscribebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$11\r\nunsubscribe\r\n"));
    this->psubscribebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$10\r\npsubscribe\r\n"));
    this->punsubscribebulk = this->coordinator->getFlyObjStringFactory()->getObject(new std::string("$12\r\npunsubscribe\r\n"));
}

SharedObjects::~SharedObjects() {
    this->pong = nullptr;
    int size = this->mbulkHeaders.size();
    for (int i = 0; i < size; i++) {
        this->mbulkHeaders[i] = nullptr;
    }
    this->mbulkHeaders.clear();
}

int SharedObjects::getSharedMbulkHeadersSize() const {
    return this->mbulkHeaders.size();
}

std::shared_ptr<FlyObj> SharedObjects::getMbulkHeader(int num) const {
    if (num >= this->mbulkHeaders.size() || num < 0) {
        return nullptr;
    }

    return this->mbulkHeaders.at(num);
}

std::shared_ptr<FlyObj> SharedObjects::getPong() const {
    return this->pong;
}

std::shared_ptr<FlyObj> SharedObjects::getPing() const {
    return this->ping;
}

std::shared_ptr<FlyObj> SharedObjects::getOk() const {
    return this->ok;
}

int SharedObjects::getSelectSize() const {
    return this->select.size();
}

std::shared_ptr<FlyObj> SharedObjects::getSelect(int num) const {
    if (num >= this->select.size() || num < 0) {
        return nullptr;
    }

    return this->select.at(num);
}

std::shared_ptr<FlyObj> SharedObjects::getMessagebulk() const {
    return messagebulk;
}

std::shared_ptr<FlyObj> SharedObjects::getPmessagebulk() const {
    return pmessagebulk;
}

std::shared_ptr<FlyObj> SharedObjects::getSubscribebulk() const {
    return subscribebulk;
}

std::shared_ptr<FlyObj> SharedObjects::getUnsubscribebulk() const {
    return unsubscribebulk;
}

std::shared_ptr<FlyObj> SharedObjects::getPsubscribebulk() const {
    return psubscribebulk;
}

std::shared_ptr<FlyObj> SharedObjects::getPunsubscribebulk() const {
    return punsubscribebulk;
}
