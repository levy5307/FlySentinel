//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_ABSTRACTFLYOBJFACTORY_H
#define FLYSENTINEL_ABSTRACTFLYOBJFACTORY_H

#include "../../flyObj/FlyObj.h"

/**
 *  抽象工厂： FlyObj有两个变化维度：type和encoding，目前每个type只对应一个encoding，后续再扩展
 *           其中type基本固定，encoding后续会扩展，所以以encoding纬度作为具体工厂扩展，
 *           后续增加encoding只需要扩展一个工厂即可，无需修改工厂。
 *
 **/
class AbstractFlyObjFactory {
public:
    virtual std::shared_ptr<FlyObj> getObject() = 0;
    virtual std::shared_ptr<FlyObj> getObject(void *ptr) = 0;
};

#endif //FLYSENTINEL_ABSTRACTFLYOBJFACTORY_H
