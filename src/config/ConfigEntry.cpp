//
// Created by levy on 2019/3/21.
//

#include "ConfigEntry.h"

ConfigEntry::ConfigEntry() {

}

ConfigEntry::ConfigEntry(configProc proc, int arity) {
    this->proc = proc;
    this->arity = arity;
}

configProc ConfigEntry::getProc() const {
    return proc;
}

int ConfigEntry::getArity() const {
    return arity;
}

void ConfigEntry::setArity(int arity) {
    ConfigEntry::arity = arity;
}

