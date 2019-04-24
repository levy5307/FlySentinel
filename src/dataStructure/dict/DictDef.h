//
// Created by 赵立伟 on 2018/9/22.
//

#ifndef FLYDB_DICTDEF_H
#define FLYDB_DICTDEF_H

const uint32_t HASH_TABLE_INITIAL_SIZE = 20;      /** hash table初始大小 */
const uint8_t NEED_FORCE_REHASH_RATIO = 5;        /** 强制rehash的触发比例 */
const uint8_t HASHTABLE_MIN_FILL = 10;            /** 小于10%时，缩容 */

#endif //FLYDB_DICTDEF_H
