//
// Created by levy on 2019/3/17.
//

#ifndef FLYDB_ABSTRACTSHAREDOBJECT_H
#define FLYDB_ABSTRACTSHAREDOBJECT_H

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
};

#endif //FLYDB_ABSTRACTSHAREDOBJECT_H
