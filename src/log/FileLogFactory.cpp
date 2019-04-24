//
// Created by 赵立伟 on 2018/12/4.
//

#include "FileLogFactory.h"
#include "FileLogHandler.h"

void FileLogFactory::init(char *logfile,
                          int syslogEnabled,
                          int verbosity) {
    FileLogHandler::init(logfile, syslogEnabled, verbosity);
}

AbstractLogHandler* FileLogFactory::getLogger() {
    return FileLogHandler::getInstance();
}
