#include "EventLoopThread.h"

#include <boost/bind.hpp>

#include "EventLoop.h"

using namespace muduo;

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
      exiting(false),
      thread_(boost::bind(&EventLoopThread::threadFunc, this)),
      mutex_(),
      cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
  exiting = true;
  loop_->quit();
  thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  // 让线程开始执行主函数
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.wait();
    }
  }

  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();
  // assert(exiting_);
}