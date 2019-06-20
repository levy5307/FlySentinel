//
// Created by levy on 2019/5/27.
//

#ifndef FLYSENTINEL_SCRIPTJOBDEF_H
#define FLYSENTINEL_SCRIPTJOBDEF_H

const int SENTINEL_SCRIPT_NONE = 0;
const int SENTINEL_SCRIPT_RUNNING = 1;
const int SENTINEL_SCRIPT_RETRY_DELAY = 30000;     /** 30 seconds between retries. */
const int SENTINEL_SCRIPT_MAX_RUNTIME = 60000;     /** 60s最大运行时间 */

#endif //FLYSENTINEL_SCRIPTJOBDEF_H
