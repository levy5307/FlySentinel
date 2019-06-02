//
// Created by levy on 2019/4/22.
//

#include <iostream>
#include <zconf.h>

#include "utils/MiscTool.h"
#include "log/FileLogFactory.h"
#include "coordinator/interface/AbstractCoordinator.h"
#include "coordinator/Coordinator.h"

bool canResize = true;
AbstractLogFactory *logFactory = new FileLogFactory();
MiscTool *miscTool = MiscTool::getInstance();
AbstractCoordinator *coordinator = NULL;

int main(int argc, char **argv) {
    coordinator = new Coordinator();
    coordinator->getEventLoop()->eventMain();
}
