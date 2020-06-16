#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Channel.h"
#include "Socket.h"

namespace muduo {
class EventLoop;
class InetAddress;

// 连接器，用于管理连接建立事件
class Acceptor : boost::noncopyable {
 public:
  typedef boost::function<void(int sockfd, const InetAddress&)>
      NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr);
  // 设置 accept 返回后的 callback
  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  // Socket.listen()
  // Channel.enableReading()
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
};

};  // namespace muduo

#endif