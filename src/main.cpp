//
// Created by levy on 2019/4/22.
//

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
    /** 初始化随机种子 */
    srand((unsigned)time(NULL) ^ getpid());
    coordinator = new Coordinator();
    coordinator->getEventLoop()->eventMain();
}
