//
// Created by levy on 2019/3/17.
//

#ifndef FLYSENTINEL_SHAREDOBJECTS_H
#define FLYSENTINEL_SHAREDOBJECTS_H


#include "SharedObjectDef.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../coordinator/interface/AbstractSharedObjects.h"

class SharedObjects : public AbstractSharedObjects {
public:
    SharedObjects(AbstractCoordinator *coordinator);
    ~SharedObjects();
    int getSharedMbulkHeadersSize() const;
    std::shared_ptr<FlyObj> getMbulkHeader(int num) const;
    std::shared_ptr<FlyObj> getPong() const;
    std::shared_ptr<FlyObj> getPing() const;
    std::shared_ptr<FlyObj> getOk() const;
    int getSelectSize() const;
    std::shared_ptr<FlyObj> getSelect(int num) const;
    std::shared_ptr<FlyObj> getMessagebulk() const;
    std::shared_ptr<FlyObj> getPmessagebulk() const;
    std::shared_ptr<FlyObj> getSubscribebulk() const;
    std::shared_ptr<FlyObj> getUnsubscribebulk() const;
    std::shared_ptr<FlyObj> getPsubscribebulk() const;
    std::shared_ptr<FlyObj> getPunsubscribebulk() const;

private:
    /** 共享multi bulk len字段，其格式为: "*<value>\r\n" */
    std::vector<std::shared_ptr<FlyObj> > mbulkHeaders;
    std::shared_ptr<FlyObj> pong;
    std::shared_ptr<FlyObj> ping;
    std::shared_ptr<FlyObj> ok;
    std::vector<std::shared_ptr<FlyObj>> select;
    std::shared_ptr<FlyObj> messagebulk;
    std::shared_ptr<FlyObj> pmessagebulk;
    std::shared_ptr<FlyObj> subscribebulk;
    std::shared_ptr<FlyObj> unsubscribebulk;
    std::shared_ptr<FlyObj> psubscribebulk;
    std::shared_ptr<FlyObj> punsubscribebulk;

    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_SHAREDOBJECTS_H
