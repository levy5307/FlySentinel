//
// Created by levy on 2019/3/21.
//

#include <iostream>
#include <syslog.h>
#include "BaseConfigReader.h"
#include "../../dataStructure/dict/Dict.cpp"
#include "../../log/LogDef.h"
#include "../../def.h"

configMap loglevelMap[] = {
        {"debug",   LL_DEBUG},
        {"verbose", LL_VERBOSE},
        {"notice",  LL_NOTICE},
        {"warning", LL_WARNING},
        {nullptr, 0}
};

configMap syslogFacilityMap[] = {
        {"user",    LOG_USER},
        {"local0",  LOG_LOCAL0},
        {"local1",  LOG_LOCAL1},
        {"local2",  LOG_LOCAL2},
        {"local3",  LOG_LOCAL3},
        {"local4",  LOG_LOCAL4},
        {"local5",  LOG_LOCAL5},
        {"local6",  LOG_LOCAL6},
        {"local7",  LOG_LOCAL7},
        {nullptr, 0}
};

BaseConfigReader::BaseConfigReader() {
    configCache = new ConfigCache();

    /** init config entry */
    this->initConfigEntry();
}

void BaseConfigReader::initConfigEntry() {
    configEntryTable = new Dict<std::string, std::shared_ptr<ConfigEntry> >();
    configEntryTable->addEntry("logfile", std::shared_ptr<ConfigEntry>(new ConfigEntry(logFileConfigProc, 2)));
    configEntryTable->addEntry("syslog-enable", std::shared_ptr<ConfigEntry>(new ConfigEntry(syslogEnableConfigProc, 2)));
    configEntryTable->addEntry("syslog-ident", std::shared_ptr<ConfigEntry>(new ConfigEntry(syslogIdentConfigProc, 2)));
    configEntryTable->addEntry("loglevel", std::shared_ptr<ConfigEntry>(new ConfigEntry(logLevelConfigProc, 2)));
    configEntryTable->addEntry("syslog-facility", std::shared_ptr<ConfigEntry>(new ConfigEntry(syslogFacilityConfigProc, 2)));
}

BaseConfigReader::~BaseConfigReader() {
    delete configEntryTable;
}

void BaseConfigReader::parseConfig(std::vector<std::string> &words) {
    /** 查找相应的ConfigEntry */
    DictEntry<std::string, std::shared_ptr<ConfigEntry> >* dictEntry = this->configEntryTable->findEntry(words[0]);
    if (nullptr == dictEntry) {
        return;
    }

    /** 字段数量不满足要求 */
    if (words.size() < dictEntry->getVal()->getArity()) {
        return;
    }

    /** 处理配置读取 */
    dictEntry->getVal()->getProc()(this->configCache, words);
}

int configMapGetValue(configMap *config, const char *name) {
    while (config->name != nullptr) {
        if (!strcasecmp(config->name, name)) {
            return config->value;
        }
        config++;
    }
    return INT_MIN;
}

void logFileConfigProc(ConfigCache* configCache, std::vector<std::string> &words) {
    free(configCache->getLogfile());

    char *logfile = strdup(words[1].c_str());
    if ('\0' != logfile[0]) {       // log文件名不为空
        // 尝试打开一次，查看是否可以正常打开
        FILE *logfd = fopen(logfile, "a");
        if (nullptr == logfd) {
            std::cout << "Can not open log file: " << logfile << std::endl;
            exit(1);
        }
        fclose(logfd);
    }

    configCache->setLogfile(logfile);
}

void syslogEnableConfigProc(ConfigCache* configCache, std::vector<std::string> &words) {
    int syslogEnabled;
    if (-1 == (syslogEnabled = miscTool->yesnotoi(words[1].c_str()))) {
        std::cout << "syslog-enabled must be 'yes(YES)' or 'no(NO)'"
                  << std::endl;
        exit(1);
    }
    configCache->setSyslogEnabled(syslogEnabled);
}

void syslogIdentConfigProc(ConfigCache* configCache, std::vector<std::string> &words) {
    if (configCache->getSyslogIdent()) {
        free(configCache->getSyslogIdent());
    }
    configCache->setSyslogIdent(strdup(words[1].c_str()));
}

void logLevelConfigProc(ConfigCache* configCache, std::vector<std::string> &words) {
    int verbosity = configMapGetValue(loglevelMap, words[1].c_str());
    if (INT_MIN == verbosity) {
        std::cout << "Invalid log level. "
                     "Must be one of debug, verbose, notice, warning"
                  << std::endl;
        exit(1);
    }
    configCache->setVerbosity(verbosity);
}

void syslogFacilityConfigProc(ConfigCache* configCache, std::vector<std::string> &words) {
    int syslogFacility =
            configMapGetValue(syslogFacilityMap, words[1].c_str());
    if (INT_MIN == syslogFacility) {
        std::cout << "Invalid log facility. "
                     "Must be one of USER or between LOCAL0-LOCAL7"
                  << std::endl;
        exit(1);
    }
    configCache->setSyslogFacility(syslogFacility);
}

