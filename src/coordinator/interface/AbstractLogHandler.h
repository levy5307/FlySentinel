//
// Created by 赵立伟 on 2018/12/3.
//

#ifndef FLYSENTINEL_ABSTRACTLOGHANDLER_H
#define FLYSENTINEL_ABSTRACTLOGHANDLER_H

class AbstractLogHandler {
public:
    virtual void logDebug(const char *fmt, ...) = 0;

    virtual void logVerbose(const char *fmt, ...) = 0;

    virtual void logNotice(const char *fmt, ...) = 0;

    virtual void logWarning(const char *fmt, ...) = 0;

    virtual void log(int level, const char *fmt, ...) = 0;

};

#endif //FLYSENTINEL_ABSTRACTLOGHANDLER_H
