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

private:
    /** 共享multi bulk len字段，其格式为: "*<value>\r\n" */
    std::vector<std::shared_ptr<FlyObj> > mbulkHeaders;
    std::shared_ptr<FlyObj> pong;
    std::shared_ptr<FlyObj> ping;
    std::shared_ptr<FlyObj> ok;
    std::vector<std::shared_ptr<FlyObj> > select;

    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_SHAREDOBJECTS_H
