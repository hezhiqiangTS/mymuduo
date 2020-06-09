#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

#include "../muduo/base/Thread.h"
#include "../muduo/base/Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace muduo {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : boost ::noncopyable {
 public:
  typedef boost::function<void()> Functor;
  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  Timestamp pollReturnTime() const { return pollReturnTime_; }

  // Call cb in thread which owns thread_loop
  // Thread Safe
  void runInLoop(const Functor& cb);
  void queueInLoop(const Functor& cb);

  // 添加一个 Timer(TimerCallback, time, 0)
  TimerId runAt(const Timestamp& time, const TimerCallback& cb);
  // 添加一个 Timer(TimerCallback, now + delay, 0)
  TimerId runAfter(double delay, const TimerCallback& cb);
  // 添加一个 Timer(TimerCallback, now + interval, interval)
  TimerId runEvery(double interval, const TimerCallback& cb);

  void wakeup();
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
  void handleRead();
  void doPendingFunctors();
  void abortNotInLoopThread();

  typedef std::vector<Channel*> ChannelList;

  bool looping_;
  bool quit_;
  bool callingPendingFunctors_;

  const pid_t threadId_;
  Timestamp pollReturnTime_;
  boost::scoped_ptr<Poller> poller_;
  boost::scoped_ptr<TimerQueue> timerQueue_;

  int wakeupFd_;
  boost::scoped_ptr<Channel> wakeupChannel_;

  MutexLock mutex_;
  std::vector<Functor> pendingFunctors_;
  ChannelList activeChannels_;
};
}  // namespace muduo
#endif