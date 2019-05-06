//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYSENTINEL_ABSTRACTCONFIGREADER_H
#define FLYSENTINEL_ABSTRACTCONFIGREADER_H

#include "../ConfigCache.h"
#include "../ConfigEntry.h"

template<class KEY, class VAL>
class Dict;

struct configMap {
    const char *name;
    const int value;
};

class BaseConfigReader {
public:
    BaseConfigReader();
    virtual ~BaseConfigReader();
    virtual ConfigCache* loadConfig() = 0;
    void parseConfig(std::vector<std::string> &words);

protected:
    void initConfigEntry();

    ConfigCache *configCache;
    Dict<std::string, std::shared_ptr<ConfigEntry> >* configEntryTable;
};

#endif //FLYSENTINEL_ABSTRACTCONFIGREADER_H
