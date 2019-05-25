//
// Created by levy on 2019/3/17.
//

#ifndef FLYSENTINEL_ABSTRACTSHAREDOBJECT_H
#define FLYSENTINEL_ABSTRACTSHAREDOBJECT_H

#include <memory>

class FlyObj;

class AbstractSharedObjects {
public:
    virtual ~AbstractSharedObjects() {}
    virtual int getSharedMbulkHeadersSize() const = 0;
    virtual std::shared_ptr<FlyObj> getMbulkHeader(int num) const = 0;
    virtual std::shared_ptr<FlyObj> getPong() const = 0;
    virtual std::shared_ptr<FlyObj> getPing() const = 0;
    virtual std::shared_ptr<FlyObj> getOk() const = 0;
    virtual int getSelectSize() const = 0;
    virtual std::shared_ptr<FlyObj> getSelect(int num) const = 0;
    virtual std::shared_ptr<FlyObj> getMessagebulk() const = 0;
    virtual std::shared_ptr<FlyObj> getPmessagebulk() const = 0;
    virtual std::shared_ptr<FlyObj> getSubscribebulk() const = 0;
    virtual std::shared_ptr<FlyObj> getUnsubscribebulk() const = 0;
    virtual std::shared_ptr<FlyObj> getPsubscribebulk() const = 0;
    virtual std::shared_ptr<FlyObj> getPunsubscribebulk() const = 0;
};

#endif //FLYSENTINEL_ABSTRACTSHAREDOBJECT_H
