#include "TcpServer.h"

#include <stdio.h>

#include <boost/bind.hpp>

#include "../muduo/base/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"

using namespace muduo;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(listenAddr.toHostPort()),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1) {
  acceptor_->setNewConnectionCallback(
      boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
  if (!started_) {
    started_ = true;
  }

  if (!acceptor_->listenning()) {
    loop_->runInLoop(boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

// 当listenfd就绪后的回调函数
// 创建 TcpConnection 对象
// 将 TcpConnection 对象加入 connectionmap
// 设置 connectioncallback
// 设置 messagecallback
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
           << connName << "] from " << peerAddr.toHostPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(
      new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->connectEstablished();
}