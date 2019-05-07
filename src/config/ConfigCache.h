//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYSENTINEL_FILECONFIG_H
#define FLYSENTINEL_FILECONFIG_H

#include <string>
#include <vector>

struct ConfigCache {
public:
    ConfigCache();

    /** getter / setter*/
    int getVerbosity() const;
    void setVerbosity(int verbosity);
    char *getLogfile() const;
    void setLogfile(char *logfile);
    int getSyslogEnabled() const;
    void setSyslogEnabled(int syslogEnabled);
    char *getSyslogIdent() const;
    void setSyslogIdent(char *syslogIdent);
    int getSyslogFacility() const;
    void setSyslogFacility(int syslogFacility);

    const std::vector <std::string> &getBindAddrs() const;
    void addBindAddr(const std::string &bindAddr);
    const char *getUnixsocket() const;
    void setUnixsocket(const char *unixsocket);
    mode_t getUnixsocketperm() const;
    void setUnixsocketperm(mode_t unixsocketperm);
    int getTcpKeepAlive() const;
    void setTcpKeepAlive(int tcpKeepAlive);
    int getPort() const;
    void setPort(int port);


private:
    /**
     * log相关
     */
    int verbosity;                                  // log level in log file
    char *logfile;                                  // log file
    int syslogEnabled;                              // 是否开启log
    char *syslogIdent;                              // log标记
    int syslogFacility;

    /**
     * 网络相关
     * */
    std::vector<std::string> bindAddrs;             // 绑定地址
    const char *unixsocket;                         // UNIX socket path
    mode_t unixsocketperm;                          // UNIX socket permission
    int tcpKeepAlive;
    int port;                                       // tcp listening port
};

#endif //FLYSENTINEL_FILECONFIG_H
