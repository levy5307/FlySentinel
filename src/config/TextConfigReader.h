//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYSENTINEL_CONFIGREADER_H
#define FLYSENTINEL_CONFIGREADER_H

#include <string>
#include "base/BaseConfigReader.h"
#include "../utils/MiscTool.h"
#include "../def.h"
#include "ConfigCache.h"

class TextConfigReader : public BaseConfigReader {
public:
    TextConfigReader(std::string &configfile);
    ~TextConfigReader();
    ConfigCache* loadConfig();

private:
    void loadConfigFromString(const std::string& config);
    void loadConfigFromLineString(const std::string &line);

    FILE *fp;
    MiscTool *miscTool;
};

#endif //FLYSENTINEL_CONFIGREADER_H
