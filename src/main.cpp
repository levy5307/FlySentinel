//
// Created by levy on 2019/4/22.
//

#include <iostream>
#include <zconf.h>

#include "utils/MiscTool.h"
#include "log/FileLogFactory.h"

AbstractLogFactory *logFactory = new FileLogFactory();
MiscTool *miscTool = MiscTool::getInstance();

int main(int argc, char **argv) {
    while(1) {
        std::cout << "Hello, flySentinel. Wish you be better!" << std::endl;
        sleep(1);
    }
}
