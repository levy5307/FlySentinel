//
// Created by 赵立伟 on 2019/04/24.
//
#include "ConfigCache.h"
#include "../def.h"

ConfigCache::ConfigCache() {
    this->port = CONFIG_DEFAULT_SERVER_PORT;
    // unix domain socket
    this->unixsocket = NULL;
    this->unixsocketperm = CONFIG_DEFAULT_UNIX_SOCKET_PERM;

    // keep alive
    this->tcpKeepAlive = CONFIG_DEFAULT_TCP_KEEPALIVE;

    // log相关
    this->logfile = strdup(CONFIG_DEFAULT_LOGFILE.c_str());
    this->verbosity = CONFIG_DEFAULT_VERBOSITY;
    this->syslogEnabled = CONFIG_DEFAULT_SYSLOG_ENABLED;
    this->syslogIdent = strdup(CONFIG_DEFAULT_SYSLOG_IDENT.c_str());
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

const std::vector <std::string> &ConfigCache::getBindAddrs() const {
    return this->bindAddrs;
}

void ConfigCache::addBindAddr(const std::string &bindAddr) {
    this->bindAddrs.push_back(bindAddr);
}

const char *ConfigCache::getUnixsocket() const {
    return this->unixsocket;
}

void ConfigCache::setUnixsocket(const char *unixsocket) {
    this->unixsocket = unixsocket;
}

mode_t ConfigCache::getUnixsocketperm() const {
    return this->unixsocketperm;
}

void ConfigCache::setUnixsocketperm(mode_t unixsocketperm) {
    this->unixsocketperm = unixsocketperm;
}

int ConfigCache::getTcpKeepAlive() const {
    return this->tcpKeepAlive;
}

void ConfigCache::setTcpKeepAlive(int tcpKeepAlive) {
    this->tcpKeepAlive = tcpKeepAlive;
}

int ConfigCache::getPort() const {
    return this->port;
}

void ConfigCache::setPort(int port) {
    this->port = port;
}
