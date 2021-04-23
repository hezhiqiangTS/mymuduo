/*
 * 每个线程只能包含一个EventLoop对象
 *
 * */


#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

#include "../muduo/base/Thread.h"
namespace muduo {

class Channel;
class Poller;

class EventLoop : boost ::noncopyable {
 public:
  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  void updateChannel(Channel* channel);

  // 保证当前线程与创建EventLoop的线程是一个线程
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const {
    return threadId_ == muduo::CurrentThread::tid();
  }

 private:
  void abortNotInLoopThread();
  typedef std::vector<Channel*> ChannelList;

  bool looping_;
  bool quit_;
  const pid_t threadId_;
  boost::scoped_ptr<Poller> poller_;
  ChannelList activeChannels_;
};
}  // namespace muduo
#endif