//
// Created by levy on 2019/3/21.
//

#ifndef FLYSENTINEL_CONFIGENTRY_H
#define FLYSENTINEL_CONFIGENTRY_H

#include <string>

class BaseConfigReader;
class ConfigCache;

typedef void (*configProc)(ConfigCache*, std::vector<std::string>&);
class ConfigEntry {
public:
    ConfigEntry();
    ConfigEntry(configProc proc, int arity);

    configProc getProc() const;
    int getArity() const;
    void setArity(int arity);

private:
    configProc proc;                         /** 处理函数 */
    int arity = 0;                           /** 配置字段数量 */
};


#endif //FLYSENTINEL_CONFIGENTRY_H
