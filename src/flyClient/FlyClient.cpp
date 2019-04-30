//
// Created by 赵立伟 on 2018/10/18.
//

#include "FlyClient.h"
#include "../net/NetDef.h"
#include "ClientDef.h"
#include "../io/MemFio.h"

FlyClient::FlyClient(int fd,
                     const AbstractCoordinator *coordinator,
                     AbstractFlyDB *flyDB,
                     time_t nowt) {
    this->coordinator = coordinator;
    this->id = 0;
    this->fd = fd;
    this->name = NULL;
    this->flags = 0;
    this->argc = 0;
    this->argv = NULL;
    this->cmd = NULL;
    this->authentiated = 0;
    this->createTime = this->lastInteractionTime = nowt;
    this->softLimitTime = 0;
    this->bufpos = 0;
    this->replyBytes = 0;
    this->reqType = 0;
    this->multiBulkLen = 0;
    this->bulkLen = 0;
    this->sendLen = 0;
    this->flyDB = flyDB;
    this->slaveIP[0] = '\0';
}

FlyClient::~FlyClient() {
    this->freeArgv();
    this->clearOutputBuffer();
}

int FlyClient::getFd() const {
    return fd;
}

void FlyClient::setFd(int fd) {
    this->fd = fd;
}

std::shared_ptr<FlyObj> FlyClient::getName() const {
    return name;
}

void FlyClient::setName(std::shared_ptr<FlyObj> name) {
    this->name = name;
}

int FlyClient::getFlags() const {
    return flags;
}

void FlyClient::setFlags(int flags) {
    this->flags = flags;
}

void FlyClient::addFlag(int flag) {
    this->flags |= flag;
}

void FlyClient::delFlag(int flag) {
    this->flags &= ~flag;
}

std::shared_ptr<FlyObj> *FlyClient::getArgv() const {
    return this->argv;
}

void FlyClient::freeArgv() {
    if (NULL != this->argv) {
        for (int i = 0; i < this->argc; i++) {
            this->argv[i] = NULL;
        }
        delete[] this->argv;
        this->argv = NULL;
    }
    this->argc = 0;
}

void FlyClient::allocArgv(int64_t count) {
    this->freeArgv();
    this->argv = new std::shared_ptr<FlyObj> [count];
}

void FlyClient::addArgv(std::shared_ptr<FlyObj> obj) {
    this->argv[this->argc++] = obj;
}

int FlyClient::getArgc() const {
    return argc;
}

void FlyClient::setArgc(int argc) {
    this->argc = argc;
}

int FlyClient::getAuthentiated() const {
    return authentiated;
}

void FlyClient::setAuthentiated(int authentiated) {
    this->authentiated = authentiated;
}

time_t FlyClient::getCreateTime() const {
    return createTime;
}

void FlyClient::setCreateTime(time_t createTime) {
    this->createTime = createTime;
}

time_t FlyClient::getLastInteractionTime() const {
    return lastInteractionTime;
}

void FlyClient::setLastInteractionTime(time_t lastInteractionTime) {
    this->lastInteractionTime = lastInteractionTime;
}

time_t FlyClient::getSoftLimitTime() const {
    return softLimitTime;
}

void FlyClient::setSoftLimitTime(time_t softLimitTime) {
    this->softLimitTime = softLimitTime;
}

const std::list<std::string*> &FlyClient::getReply() const {
    return this->replies;
}

void FlyClient::replyPopFront() {
    this->replyBytes -= this->replies.front()->size();
    this->replies.pop_front();
    if (0 == this->replies.size() && 0 != this->replyBytes) {
        coordinator->getLogHandler()->logVerbose("The reply size is zero, "
                                     "but replyBytes is not: %d",
                                     this->replyBytes);
        this->replyBytes = 0;
    }
}

void FlyClient::setReply(const std::list<std::string*> &replies) {
    this->replies = replies;
}

const std::string &FlyClient::getQueryBuf() const {
    return queryBuf;
}

void FlyClient::addToQueryBuf(const std::string &str) {
    this->queryBuf += str;
}

void FlyClient::addToPendingQueryBuf(const std::string &str) {
    this->pendingQueryBuf += str;
}

int FlyClient::getQueryBufSize() const {
    return this->queryBuf.length();
}

uint64_t FlyClient::getId() const {
    return this->id;
}

const char *FlyClient::getBuf() const {
    return this->buf;
}

void FlyClient::clearBuf(){
    this->setBufpos(0);
    this->setSendLen(0);
}

void FlyClient::setBuf(const char* buf, int size) {
    memcpy(this->buf, buf, size);
}

bool FlyClient::bufSendOver() {
    return this->sendLen == this->bufpos;
}

void FlyClient::setId(uint64_t id) {
    this->id = id;
}

char FlyClient::getFirstQueryChar() {
    return this->queryBuf[0];
}

bool FlyClient::isMultiBulkType() {
    return PROTO_REQ_MULTIBULK == this->reqType;
}

int32_t FlyClient::getMultiBulkLen() const {
    return this->multiBulkLen;
}

void FlyClient::setMultiBulkLen(int32_t multiBulkLen) {
    this->multiBulkLen = multiBulkLen;
}

void FlyClient::setQueryBuf(const std::string &queryBuf) {
    this->queryBuf = queryBuf;
}

void FlyClient::trimQueryBuf(int begin, int end) {
    std::string sub;
    if (-1 == end) {
        sub = queryBuf.substr(begin);
    } else {
        sub = queryBuf.substr(begin, end - begin + 1);
    }

    setQueryBuf(sub);
}

int FlyClient::processInputBuffer() {
    /** 保存处理命令前的offset */
    int64_t prevOffset = this->getReploff();

    while (this->getQueryBufSize() > 0) {
        // 第一个字符是'*'代表是整体multibulk串;
        // reqtype=multibulk代表是上次读取已经处理了部分的multibulk
        if ('*' == this->getFirstQueryChar() || this->isMultiBulkType()) {
            // 读取失败，直接返回，不做命令处理, inline buffer同理
            if (-1 == processMultiBulkBuffer()) {
                return -1;
            }
        } else {
            if (-1 == processInlineBuffer()) {
                return -1;
            }
        }
    }

    /**
     * 设置repl off
     *  因为处理的时候有可能没有将pending buf中的所有数据都处理完，而存留了部分，
     *  所以在处理repl off时需要根据read repl off - 留存的数据size
     **/
    if (this->getFlags() & CLIENT_MASTER) {
        this->setReploff(this->getReadReploff() - this->getPendingBuf().size());
    }

    /** 处理命令 */
    bool isWrite = coordinator->getFlyServer()->dealWithCommand(this->getFd());

    /** 命令传播 */
    if (isWrite > 0) {
        coordinator->getFlyServer()->propagate(this->getFd(), prevOffset);
    }
}

int FlyClient::writeToClient(int handlerInstalled) {
    ssize_t onceCount = 0, totalCount = 0;
    int fd = this->getFd();

    // 循环写入
    while (!this->hasNoPending()) {
        if (0 != this->getBufpos()) {
            // 通过网络发送出去
            onceCount = write(this->getFd(),
                              this->getBuf() + this->getSendLen(),
                              this->getBufpos() - this->getSendLen());
            if (onceCount <= 0) {
                break;
            }
            this->addSendLen(onceCount);
            totalCount += onceCount;

            // 固定buf全部发送完
            if (this->bufSendOver()) {
                this->clearBuf();
            }
        } else {
            std::string* reply = this->getReply().front();
            int replyLen = reply->size();
            int sentLen = this->getSendLen();

            onceCount = write(fd, reply->c_str() + sentLen, replyLen - sentLen);
            if (onceCount <= 0) {
                break;
            }
            this->addSendLen(onceCount);
            totalCount += onceCount;

            // 该reply发送完毕
            if (this->getSendLen() == replyLen) {
                this->setSendLen(0);
                this->replyPopFront();
            }
        }

        // 达到最大发送长度
        if (totalCount > NET_MAX_WRITES_PER_EVENT) {
            break;
        }
    }

    // 如果写入过程出错，删除flyClient
    if (onceCount <= 0 && EAGAIN != errno) {
        coordinator->getFlyServer()->freeClientAsync(this->getFd());
        close(fd);
        return -1;
    }

    // 统计服务端总共发送的字节数
    coordinator->getFlyServer()->addToStatNetInputBytes(totalCount);

    // 如果全部发送完
    if (this->hasNoPending()) {
        // 删除hanlder
        this->setSendLen(0);
        if (handlerInstalled) {
            coordinator->getEventLoop()->deleteFileEvent(fd, ES_WRITABLE);
        }

        if (this->getFlags() & CLIENT_CLOSE_AFTER_REPLY) {
            coordinator->getFlyServer()->freeClientAsync(this->getFd());
            close(fd);
            return -1;
        }
    }

    // 不管是否发送完，都返回1
    return 1;
}

int FlyClient::processInlineBuffer() {
    size_t pos = this->getQueryBuf().find("\r\n");
    if (pos == this->getQueryBuf().npos) {     // 没有找到
        if (this->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            this->addReplyError("Protocol error: too big mbulk count string");
            setProtocolError("too big mbulk count string", 0);
        }
        return -1;
    }

    std::vector<std::string> words;
    std::string subStr = this->getQueryBuf().substr(0, pos);
    miscTool->spiltString(subStr, " ", words);
    if (0 == words.size()) {
        return -1;
    }

    /** 分配argv空间: 先清空再分配 */
    this->allocArgv(words.size());

    // 设置参数列表
    for (auto item : words) {
        this->addArgv(coordinator->getFlyObjStringFactory()->getObject(new std::string(item)));
    }

    // 裁剪输入缓冲区
    this->trimQueryBuf(pos + 2, -1);
    return 1;
}

/**
 * 用于处理RESP协议的请求。将client的query buffer拆解到client的argv中。
 * 如果成功，返回0；
 * 否则返回-1，有如下两种情况：
 *  1.当前的query buffer中还不足以解析出一个完整的command，需要等待下次读取完
 *  2.当前协议解析失败，此时需要中断和客户端的连接
 */
int FlyClient::processMultiBulkBuffer() {
    size_t pos = 0;
    this->setReqType(PROTO_REQ_MULTIBULK);

    // multi bulk等于0，代表是新的multibulk字符串，
    // 否则就是未读完的部分(未读完的部分不用重新读取multi bulk len)
    if (0 == this->getMultiBulkLen()) {
        // 如果获取multi bulk len失败，返回-1
        if (-1 == analyseMultiBulkLen(pos)) {
            return -1;
        }
    }

    // 解析multi bulk
    if (-1 == analyseMultiBulk(pos)) {
        return -1;
    }

    // 所有bulk读取完，重置如下字段，表示这次multi bulk处理完
    this->setMultiBulkLen(0);
    this->setReqType(0);
    return 1;
}

int FlyClient::analyseMultiBulkLen(size_t &pos) {
    pos = this->getQueryBuf().find("\r\n");
    if (pos == this->getQueryBuf().npos) {     // 没有找到
        if (this->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            this->addReplyError("Protocol error: too big mbulk count string");
            setProtocolError("too big mbulk count string", 0);
        }
        return -1;
    }

    int64_t multiBulkLen = 0;
    std::string subStr = this->getQueryBuf().substr(1, pos - 1);
    int res = miscTool->string2int64(subStr, multiBulkLen);

    // 如果获取multi bulk length失败，或者其太长，协议error
    if (-1 == res || multiBulkLen > PROTO_REQ_MULTIBULK_MAX_LEN) {
        this->addReplyError("Protocol error: invalid multibulk length");
        setProtocolError("invalid mbulk count", pos);
        return -1;
    }

    pos += sizeof("\r\n") - 1;
    /**
     * 如果multi bulk len < 0, 表示null, 该multi bulk命令读取完毕
     * 此时由于client->multiBulkLen = 0, 不会执行外围函数的后续bulk解析
     */
    if (multiBulkLen < 0) {
        this->trimQueryBuf(pos, -1);
        return 1;
    }

    // 设置multi bulk len
    this->setMultiBulkLen(multiBulkLen);

    /** 分配argv空间: 先清空再分配 */
    this->allocArgv(multiBulkLen);

    // 截取输入字符串
    this->trimQueryBuf(pos, -1);
    return 1;
}

int FlyClient::analyseMultiBulk(size_t &pos) {
    int64_t multiBulkLen = this->getMultiBulkLen();
    for (int i = this->getArgc(); i < multiBulkLen; i++) {
        if (-1 == analyseBulk()) {
            return -1;
        }
    }
}

int FlyClient::analyseBulk() {
    // 如果字符串为空，说明需要通过下次来读取，直接返回(但并不是协议错误)
    if (0 == this->getQueryBuf().size()) {
        return -1;
    }

    size_t pos = 0;
    if ('$' != this->getQueryBuf().at(pos)) {
        this->addReplyErrorFormat("Protocol error: expected '$', got '%c'", this->getQueryBuf().at(pos));
        setProtocolError("expected $ but got something else", pos);
        return -1;
    }

    // 获取到"\r\n"的位置
    size_t begin = pos + 1;
    pos = this->getQueryBuf().find("\r\n", begin);
    if (pos == this->getQueryBuf().npos) {     // 没有找到
        if (this->getQueryBufSize() > PROTO_INLINE_MAX_SIZE) {
            this->addReplyError("Protocol error: too big bulk count string");
            setProtocolError("too big bulk count string", 0);
        }
        return -1;
    }

    // 从client->querybuf里截取该bulk
    int64_t bulkLen = 0;
    std::string subStr = this->getQueryBuf().substr(begin, pos - begin);
    int res = miscTool->string2int64(subStr, bulkLen);
    if (-1 == res || bulkLen < 0 || bulkLen > PROTO_REQ_BULK_MAX_LEN) {
        this->addReplyError("Protocol error: invalid bulk length");
        setProtocolError("invalid bulk length", pos);
    }
    int endStrSize = sizeof("\r\n");
    begin = pos + endStrSize - 1;

    // 如果读取不全（找不到\r\n），返回-1，等待下次读取
    pos = this->getQueryBuf().find("\r\n", begin);
    if (pos == this->getQueryBuf().npos) {
        return -1;
    }

    // 如果读取全了，并且长度不对，说明是协议问题
    if (pos - begin != bulkLen) {
        this->addReplyError("Protocol error: not enough bulk space");
        setProtocolError("not enough bulk space", pos);
        return -1;
    }

    // 设置flyClient argv参数
    std::shared_ptr<FlyObj> valptr = coordinator->getFlyObjStringFactory()->getObject(
            new std::string(this->getQueryBuf().substr(begin, pos - begin)));
    this->addArgv(valptr);

    // 截取此次读取
    this->trimQueryBuf(pos + endStrSize - 1, -1);
    return 1;
}

int FlyClient::setProtocolError(char *err, size_t pos) {
    // 打印log
    char buf[256];
    snprintf(buf, sizeof(buf), "Query buffer during protocol error: '%s'", this->getQueryBuf().c_str());
    coordinator->getLogHandler()->logVerbose("Protocol error (%s) from client: %ld. %s", err, this->getId(), buf);

    // 设置回复后关闭
    this->addFlag(CLIENT_CLOSE_AFTER_REPLY);
    // 截断query buf
    this->trimQueryBuf(pos + 2, -1);
}

int64_t FlyClient::getBulkLen() const {
    return bulkLen;
}

void FlyClient::setBulkLen(int64_t bulkLen) {
    this->bulkLen = bulkLen;
}

bool FlyClient::hasNoPending() {
    return 0 == this->bufpos && 0 == this->replies.size();
}

int FlyClient::prepareClientToWrite() {
    if (this->fd < 0) {
        return -1;
    }

    /**
     * 如果当前client是主服务器，则该主服务器不接收通知。
     * 除非设置了CLIENT_MASTER_FORCE_REPLY
     **/
    if (this->flags & CLIENT_MASTER && !(this->flags & CLIENT_MASTER_FORCE_REPLY)) {
        return -1;
    }

    /**
     * 如果之前没有写入，说明write handler不存在，
     * flyserver也不存在pending client list中,
     * 需要先将其标记并放入flyserver的pending client list中
     */
    if (hasNoPending() && !(this->flags & CLIENT_PENDING_WRITE)) {
        this->coordinator->getFlyServer()->addToClientsPendingToWrite(this->getFd());
    }

    return 1;
}

bool FlyClient::IsPendingWrite() const {
    return this->flags & CLIENT_PENDING_WRITE;
}

int FlyClient::getBufpos() const {
    return bufpos;
}

void FlyClient::setBufpos(int bufpos) {
    this->bufpos = bufpos;
}

void FlyClient::addReplyRaw(const char *s) {
    size_t len = strlen(s);
    // 查看client状态是否可以写入
    if (-1 == prepareClientToWrite()) {
        return;
    }

    // 如果写入固定缓冲区失败，则向可变缓冲区写入
    if (-1 == addReplyToBuffer(s, len)) {
        addReplyToReplyList(s, len);
    }
}

void FlyClient::addReply(const char *fmt, ...) {
    char msg[LOG_MAX_LEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    this->addReplyRaw(msg);
}

void FlyClient::addReplyErrorFormat(const char *fmt, ...) {
    va_list ap;
    char msg[1024];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    int len = strlen(msg);
    // 保证msg中没有换行符, 使msg在一行内
    for (int i = 0; i < len; i++) {
        if ('\r' == msg[i] || '\n' == msg[i]) {
            msg[i] = ' ';
        }
    }

    addReplyError(msg);
}

void FlyClient::addReplyError(const char *err) {
    std::shared_ptr<MemFio> fio =
            std::shared_ptr<MemFio>(new MemFio());
    fio->writeBulkError(err);
    this->addReply(fio->getStr().c_str());
}

void FlyClient::addReplyBulkCount(int count) {
    AbstractSharedObjects *sharedObjects = coordinator->getSharedObjects();
    if (count < sharedObjects->getSharedMbulkHeadersSize()) {
        std::string *str = reinterpret_cast<std::string *>(sharedObjects->getMbulkHeader(count)->getPtr());
        this->addReply(str->c_str());
    } else {
        std::shared_ptr<MemFio> fio =
                std::shared_ptr<MemFio>(new MemFio());
        fio->writeBulkCount('*', count);
        this->addReply(fio->getStr().c_str());
    }

    return;
}

void FlyClient::addReplyBulkString(std::string str) {
    MemFio *fio = new MemFio();
    fio->writeBulkString(str);
    this->addReply(fio->getStr().c_str());
    delete fio;
}

int FlyClient::addReplyToBuffer(const char *s, size_t len) {
    // 无需写入
    if (this->flags & CLIENT_CLOSE_AFTER_REPLY) {
        return 1;
    }

    // 如果可变缓冲区中有数据，则继续往可变缓冲区写入
    if (this->replies.size() > 0) {
        return -1;
    }

    // 空间不足，无法写入
    if (sizeof(this->buf) - this->bufpos < len) {
        return -1;
    }

    // 写入固定缓冲区
    memcpy(this->buf + this->bufpos, s, len);
    this->bufpos += len;

    return 1;
}

int FlyClient::addReplyToReplyList(const char *s, size_t len) {
    // 无需写入
    if (this->flags & CLIENT_CLOSE_AFTER_REPLY) {
        return 1;
    }

    std::string *reply = this->replies.back();
    if (NULL != reply && strlen((*reply).c_str()) + len <= PROTO_REPLY_CHUNK_BYTES) {
        (*reply) += s;
    } else {
        reply = new std::string(s);
        this->replies.push_back(reply);
    }
    this->replyBytes += len;

    // todo: 检查是否达到soft limit和hard limit

    return 1;
}

int FlyClient::getReqType() const {
    return this->reqType;
}

void FlyClient::setReqType(int reqType) {
    this->reqType = reqType;
}

size_t FlyClient::getSendLen() const {
    return this->sendLen;
}

void FlyClient::setSendLen(size_t sentLen) {
    this->sendLen = sentLen;
}

void FlyClient::addSendLen(size_t sentLen) {
    this->sendLen += sentLen;
}

AbstractFlyDB *FlyClient::getFlyDB() const {
    return this->flyDB;
}

void FlyClient::setFlyDB(AbstractFlyDB *flyDB) {
    this->flyDB = flyDB;
}

/** 清除输出缓冲区: 固定 & 可变*/
void FlyClient::clearOutputBuffer() {
    /** 清除可变缓冲区 */
    for (auto reply : this->replies) {
        if (NULL != reply) {
            delete reply;
        }
    }
    this->replies.clear();

    /** 清除固定缓冲区 */
    this->clearBuf();
}

void acceptTcpHandler(const AbstractCoordinator *coordinator,
                      int fd,
                      std::shared_ptr<AbstractFlyClient> flyClient,
                      int mask) {
    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    AbstractNetHandler *netHandler = coordinator->getNetHandler();

    int cfd, cport;
    char cip[NET_IP_STR_LEN];

    for (int i = 0; i < MAX_ACCEPTS_PER_CALL; i++) {
        cfd = netHandler->tcpAccept(NULL, fd, cip, sizeof(cip), &cport);
        if (-1 == cfd) {
            return;
        }

        std::shared_ptr<AbstractFlyClient> newClient =
                flyServer->createClient(cfd);
        if (NULL == newClient) {
            coordinator->getLogHandler()->logWarning("error to create fly client");
            close(cfd);
        }
    }
}

void readQueryFromClient(const AbstractCoordinator *coordinator,
                         int fd,
                         std::shared_ptr<AbstractFlyClient> flyClient,
                         int mask) {
    if (NULL == flyClient) {
        return;
    }

    AbstractFlyServer *flyServer = coordinator->getFlyServer();
    AbstractNetHandler *netHandler = coordinator->getNetHandler();

    char buf[PROTO_IOBUF_LEN] = "\0";
    int readCnt = read(fd, buf, sizeof(buf));
    // 读取失败, 如果错误码是EAGAIN说明本次读取没数据, 则直接返回
    // 否则需要删除client
    if (-1 == readCnt) {
        if (EAGAIN == errno) {
            return;
        } else {                                // 连接异常
            flyServer->freeClient(flyClient);
            close(fd);
            return;
        }
    } else if (0 == readCnt) {                  // 关闭连接
        flyServer->freeClient(flyClient);
        close(fd);
        return;
    }

    /** 加入输入缓冲区，如果flyclient是master，则将其加入replication暂存区 */
    flyClient->addToQueryBuf(buf);
    if (flyClient->getFlags() & CLIENT_MASTER) {
        flyClient->addToPendingQueryBuf(buf);
        flyClient->addRepldbOff(readCnt);
    }

    /** 更新最新交互时间 */
    flyClient->setLastInteractionTime(flyServer->getNowt());

    /** 统计flyServer接收到的byte数量 */
    flyServer->addToStatNetInputBytes(strlen(buf));
    if (flyClient->getQueryBufSize() > flyServer->getClientMaxQuerybufLen()) {
        flyServer->freeClient(flyClient);
        close(fd);
        coordinator->getLogHandler()->logWarning("Closing client that reached max query buffer length");
        return;
    }

    /** 处理输入 */
    flyClient->processInputBuffer();
}

void sendReplyToClient(const AbstractCoordinator *coordinator,
                       int fd,
                       std::shared_ptr<AbstractFlyClient> flyClient,
                       int mask) {
    if (NULL == flyClient) {
        return;
    }

    flyClient->writeToClient(1);
}

