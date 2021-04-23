#include "EventLoop.h"

#include <assert.h>

#include "muduo/base/Logging.h"
#include "Channel.h"
#include "Poller.h"

using namespace muduo;
// __thread 关键字表示该变量为各个线程独享
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(muduo::CurrentThread::tid()),
      poller_(new Poller(this)) {
  LOG_TRACE << "EventLoop Created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {
  assert(!looping_);
  t_loopInThisThread = nullptr;
}

// 执行 EventLoop:::Loop 的线程需要拥有该 EventLoop 对象
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    poller_->poll(kPollTimeMs, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
         it != activeChannels_.end(); ++it) {
      (*it)->handleEvent();
    }
  }

  LOG_TRACE << "EventLoop " << this << " stio looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // wakeup();
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " << muduo::CurrentThread::tid();
}
