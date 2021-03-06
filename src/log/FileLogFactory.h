//
// Created by 赵立伟 on 2018/12/4.
//

#ifndef FLYSENTINEL_FILELOGFACTORY_H
#define FLYSENTINEL_FILELOGFACTORY_H


#include "../coordinator/interface/AbstractLogFactory.h"

class FileLogFactory : public AbstractLogFactory {
public:

    /**
     * 初始化函数，在调用FileLogFactory的getLogger之前必须先初始化
     */
    static void init(char *logfile, int syslogEnabled, int verbosity);

    AbstractLogHandler* getLogger();
};

#endif //FLYSENTINEL_FILELOGFACTORY_H
