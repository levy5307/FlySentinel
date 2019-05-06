//
// Created by 赵立伟 on 2018/11/18.
//

#include <syslog.h>
#include <cstdio>
#include <sys/time.h>
#include <zconf.h>
#include "FileLogHandler.h"
#include "LogDef.h"

char* FileLogHandler::logfile = NULL;
int FileLogHandler::syslogEnabled = 0;
int FileLogHandler::verbosity = 0;

FileLogHandler::FileLogHandler() {

}

FileLogHandler* FileLogHandler::getInstance() {
    static FileLogHandler *log = NULL;
    if (NULL == log) {
        log = new FileLogHandler();
    }
    return log;
}

void FileLogHandler::init(char *logfile, int syslogEnabled, int verbosity) {
    FileLogHandler::logfile = logfile;
    FileLogHandler::syslogEnabled = syslogEnabled;
    FileLogHandler::verbosity = verbosity;
}

void FileLogHandler::logRaw(int level, const char *msg) {
    const int syslogLevelMap[] = {
            LOG_DEBUG,
            LOG_INFO,
            LOG_NOTICE,
            LOG_WARNING
    };
    FILE *fp = '\0' == logfile[0] ? stdout : fopen(logfile, "a");
    if (NULL == fp) {
        return;
    }

    if (level & LL_RAW) {
        fprintf(fp, "%s", msg);
    } else {
        char buf[64];
        struct timeval tv;
        int role;
        const char *c = ".-*#";

        gettimeofday(&tv, NULL);
        int off = strftime(buf, sizeof(buf),
                "%d %b %H:%M:%S.", localtime(&tv.tv_sec));
        snprintf(buf + off, sizeof(buf) - off, "%03d", (int) tv.tv_usec / 1000);
        // todo: role
        role = 'S';
        // 日志格式： pid:role time ./-/*/# msg
        fprintf(fp, "%d:%c %s %c %s\n",
                (int) getpid(), role, buf, c[level], msg);
    }

    fflush(fp);
    if (fp != stdout) {
        fclose(fp);
    }
    if (syslogEnabled) {
        syslog(syslogLevelMap[level], "%s", msg);
    }
}

void FileLogHandler::log(int level, const char *fmt, ...) {
    if (level & 0xff < verbosity) {
        return;
    }

    char msg[LOG_MAX_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    logRaw(level, msg);
}

void FileLogHandler::logDebug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_DEBUG, fmt, ap);
    va_end(ap);
}

void FileLogHandler::logVerbose(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_VERBOSE, fmt, ap);
    va_end(ap);
}

void FileLogHandler::logNotice(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_NOTICE, fmt, ap);
    va_end(ap);
}

void FileLogHandler::logWarning(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LL_WARNING, fmt, ap);
    va_end(ap);
}

void FileLogHandler::log(int level, const char *fmt, va_list &ap) {
    char msg[LOG_MAX_LEN];
    if (level & 0xff < verbosity) {
        return;
    }

    vsnprintf(msg, sizeof(msg), fmt, ap);
    logRaw(level, msg);
}

