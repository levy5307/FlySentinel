//
// Created by 赵立伟 on 2018/12/5.
//

#include <iostream>
#include "TextConfigReader.h"
#include "ConfigCache.h"
#include "../dataStructure/dict/Dict.cpp"

TextConfigReader::TextConfigReader(std::string &configfile) {
    if ('-' == configfile[0] && '\0' == configfile[1]) {
        this->fp = stdin;
    } else {
        if (NULL == (this->fp = fopen(configfile.c_str(), "r"))) {
            exit(1);
        }
    }
}

TextConfigReader::~TextConfigReader() {
    
}

ConfigCache* TextConfigReader::loadConfig() {
    char buf[CONFIG_MAX_LINE + 1];
    std::string config;

    // 从配置文件中依次读取配置 --> config
    while (fgets(buf, CONFIG_MAX_LINE + 1, fp) != NULL) {
        config += buf;
    }

    // 读取完毕，如果不是stdin，则关闭文件
    if (fp != stdin) {
        fclose(fp);
    }

    // 加载配置
    loadConfigFromString(config);

    return this->configCache;
}

void TextConfigReader::loadConfigFromString(const std::string& config) {
    // 将文件分隔成行
    std::string delim = "\n";
    std::vector<std::string> lines;
    this->miscTool->spiltString(config, delim, lines);

    // 依次处理每行
    for (auto line : lines) {
        if (0 == line.size() || '#' == line.at(0)) {
            continue;
        }
        loadConfigFromLineString(line);
    }
}

void TextConfigReader::loadConfigFromLineString(const std::string &line) {
    // 截取words
    std::vector<std::string> words;
    this->miscTool->spiltString(line, " ", words);
    if (0 == words.size()) {
        return;
    }

    /** 解析config */
    this->parseConfig(words);

    return;
}
