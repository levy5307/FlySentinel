//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYDB_ABSTRACTLOGHANDLER_H
#define FLYDB_ABSTRACTLOGHANDLER_H

class AbstractLogHandler {
public:
    virtual void logDebug(const char *fmt, ...) = 0;

    virtual void logVerbose(const char *fmt, ...) = 0;

    virtual void logNotice(const char *fmt, ...) = 0;

    virtual void logWarning(const char *fmt, ...) = 0;

    virtual void log(int level, const char *fmt, ...) = 0;

};

#endif //FLYDB_ABSTRACTLOGHANDLER_H
