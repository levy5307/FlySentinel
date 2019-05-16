//
// Created by 赵立伟 on 2018/10/18.
//

#ifndef FLYSENTINEL_FLYCLIENT_H
#define FLYSENTINEL_FLYCLIENT_H

#include <string>
#include <list>
#include "../flyObj/FlyObj.h"
#include "../commandTable/CommandEntry.h"
#include "ClientDef.h"
#include "../coordinator/interface/AbstractFlyClient.h"
#include "../coordinator/interface/AbstractLogHandler.h"
#include "../coordinator/interface/AbstractCoordinator.h"
#include "../net/NetDef.h"

class CommandEntry;

// 对应socket的绑定接口
void acceptTcpHandler(const AbstractCoordinator *coordinator,
                      int fd,
                      void *privdata,
                      int mask);
void readQueryFromClient(const AbstractCoordinator *coordinator,
                         int fd,
                         void *privata,
                         int mask);
void sendReplyToClient(const AbstractCoordinator *coordinator,
                       int fd,
                       void *privdata,
                       int mask);

class FlyClient : public AbstractFlyClient {
public:
    FlyClient(int fd,
              const AbstractCoordinator *coordinator,
              time_t nowt);
    ~FlyClient();
    uint64_t getId() const;
    void setId(uint64_t id);
    int getFd() const;
    void setFd(int fd);

    /** 客户端标志操作 */
    int getFlags() const;
    void setFlags(int flags);
    void addFlag(int flag);
    void delFlag(int flag);

    /** 输入缓冲区相关*/
    const std::string &getQueryBuf() const;
    void setQueryBuf(const std::string &queryBuf);
    void addToQueryBuf(const std::string &str);
    void trimQueryBuf(int begin, int end);
    int getQueryBufSize() const;

    /** 命令参数相关 */
    std::shared_ptr<FlyObj> *getArgv() const;
    void freeArgv();
    void allocArgv(int64_t count);
    int getArgc() const;
    void addArgv(std::shared_ptr<FlyObj> obj);
    void setArgc(int argc);

    int getAuthentiated() const;
    void setAuthentiated(int authentiated);
    time_t getCreateTime() const;
    void setCreateTime(time_t createTime);
    time_t getLastInteractionTime() const;
    void setLastInteractionTime(time_t lastInteractionTime);
    time_t getSoftLimitTime() const;
    void setSoftLimitTime(time_t softLimitTime);
    char getFirstQueryChar();
    bool isMultiBulkType();
    int32_t getMultiBulkLen() const;
    void setMultiBulkLen(int32_t multiBulkLen);
    int64_t getBulkLen() const;
    void setBulkLen(int64_t bulkLen);
    int prepareClientToWrite();
    bool hasNoPending();
    int processInputBuffer();
    int writeToClient(int handlerInstalled);

    /** 客户端回复相关操作 */
    void replyPopFront();
    int getReqType() const;
    void setReqType(int reqType);
    size_t getSendLen() const;
    void setSendLen(size_t sentLen);
    void addSendLen(size_t sentLen);
    const std::list<std::string*> &getReply() const;
    void setReply(const std::list<std::string*> &reply);
    void addReply(const char *fmt, ...);
    void addReplyErrorFormat(const char *fmt, ...);
    void addReplyError(const char *err);
    void addReplyBulkCount(int count);
    void addReplyBulkString(std::string str);
    const char *getBuf() const;
    void clearBuf();
    void setBuf(const char* buf, int size);
    bool bufSendOver();
    bool IsPendingWrite() const;
    int getBufpos() const;
    void setBufpos(int bufpos);
    void setReplyBytes(uint64_t replyBytes);
    uint64_t getReplyBytes() const;

private:
    int addReplyToBuffer(const char *s, size_t len);
    int addReplyToReplyList(const char *s, size_t len);
    void addReplyRaw(const char *s);
    void clearOutputBuffer();
    int processInlineBuffer();
    int processMultiBulkBuffer();
    int analyseMultiBulkLen(size_t &pos);
    int analyseMultiBulk(size_t &pos);
    int analyseBulk();
    int setProtocolError(char *err, size_t pos);

    uint64_t id;
    int fd;                             // 套接字
    int flags;                          // 标志
    std::string queryBuf;               // 输入缓冲区
    std::shared_ptr<FlyObj> *argv;      // 命令参数
    int argc;
    CommandEntry* cmd;                  // 命令实现函数
    char buf[FLY_REPLY_CHUNK_BYTES];    // 固定大小输出缓冲区
    int bufpos;
    std::list<std::string *> replies;   // 可变长度输出缓冲区
    uint64_t replyBytes;
    int authentiated;                   // 是否通过了身份验证
    time_t createTime;                  // 客户端创建事件
    time_t lastInteractionTime;         // 最后一次互动时间
    time_t softLimitTime;               // 软性限制时间
    int reqType;
    int32_t multiBulkLen;               // 剩余可读的multi bulk参数数量
    int64_t bulkLen;
    size_t sendLen;                     // 记录发送长度，用于处理一次没有发送完的情况

    AbstractLogHandler *logHandler;
    const AbstractCoordinator *coordinator;
};


#endif //FLYSENTINEL_FLYCLIENT_H
