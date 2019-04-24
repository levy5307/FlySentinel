//
// Created by 赵立伟 on 2018/11/18.
//

#ifndef FLYDB_LOGDEF_H
#define FLYDB_LOGDEF_H

#define LOG_MAX_LEN    1024             // log最大长度
enum LogLevel {
    LL_DEBUG,           // 会打印出很多信息，适用于开发和测试阶段
    LL_VERBOSE,         // 冗长的：包含很多不太有用的信息，但是比debug要清爽一些
    LL_NOTICE,          // 适用于生产模式
    LL_WARNING,         // 警告信息
    LL_RAW = (1 << 10)
};

#endif //FLYDB_LOGDEF_H
