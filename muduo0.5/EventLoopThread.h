#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include <boost/noncopyable.hpp>

#include "../muduo/base/Condition.h"
#include "../muduo/base/Mutex.h"
#include "../muduo/base/Thread.h"

namespace muduo {
class EventLoop;

class EventLoopThread : boost::noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  // Main func of EventLoopThread
  /// 在栈上创建 EventLoop 对象
  // 唤醒 startLoop 函数，
  // loop_ 指向栈上创建的 EventLoop 对象
  void threadFunc();

  EventLoop* loop_;
  bool exiting;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};
};  // namespace muduo

#endif