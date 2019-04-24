//
// Created by 赵立伟 on 2019/04/24.
//
#include "ConfigCache.h"

ConfigCache::ConfigCache() {
}

int ConfigCache::getVerbosity() const {
    return this->verbosity;
}

void ConfigCache::setVerbosity(int verbosity) {
    this->verbosity = verbosity;
}

char *ConfigCache::getLogfile() const {
    return this->logfile;
}

void ConfigCache::setLogfile(char *logfile) {
    this->logfile = logfile;
}

int ConfigCache::getSyslogEnabled() const {
    return this->syslogEnabled;
}

void ConfigCache::setSyslogEnabled(int syslogEnabled) {
    this->syslogEnabled = syslogEnabled;
}

char *ConfigCache::getSyslogIdent() const {
    return this->syslogIdent;
}

void ConfigCache::setSyslogIdent(char *syslogIdent) {
    this->syslogIdent = syslogIdent;
}

int ConfigCache::getSyslogFacility() const {
    return this->syslogFacility;
}

void ConfigCache::setSyslogFacility(int syslogFacility) {
    this->syslogFacility = syslogFacility;
}
