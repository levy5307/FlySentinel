//
// Created by 赵立伟 on 2018/12/5.
//

#ifndef FLYSENTINEL_FILECONFIG_H
#define FLYSENTINEL_FILECONFIG_H

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

private:
    /**
     * log相关
     */
    int verbosity;                                  // log level in log file
    char *logfile;                                  // log file
    int syslogEnabled;                              // 是否开启log
    char *syslogIdent;                              // log标记
    int syslogFacility;
};

#endif //FLYSENTINEL_FILECONFIG_H
