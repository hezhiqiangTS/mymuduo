# 添加 EventLoop::runInLoop() 函数
`EventLoop::runInLoop(const Functor& cb)` 函数的作用：让某个 IO 线程执行回调函数 cb。

回忆 muduo0.2 中如何实现为 IO 线程添加回调函数。首先，muduo0.2 中无法直接为 IO 线程单独添加一个回调函数。但我们可以用其他方式实现类似的目的，维护一个 fd 与 CallBack 的映射，以添加`Channel`对象的方式，让 IO 线程监听该 fd 上的事件，在 poll 或epoll_wait 返回后，通过 `Channel::handleEvent()` 来执行该回调函数。

而`Channel`对象的添加是通过：`Channel::enableReading()->Channel::update()->EventLoop::updateChannel()->Poller::updateChannel()`最终将 fd 添加到 poll 监听列表中，来完成的。`muduo0.2/test2.cpp`添加自定义定时器时，`Channel::enableReading()`是由main thread调用的，而当时不涉及到多线程，main thread就是 IO thread。

实际上，由于要求执行该线程的函数必须是拥有该`EventLoop`对象的 IO 线程，所以如果当时是在多线程情况下，main thread 与 IO thread 各自有各自的 EventLoop 对象，那么我们就无法在 main thread 中为 IO thread 添加用户回调函数，只能在 IO thread 初始化的时候就设置好。

## How
现在我们知道了问题所在：之前能够完成类似功能的`Channel::enableReading()`函数不能被其他线程调用。那应该如何做？是将`Channel::enableReading()`魔改为可以跨线程调用，还是另起炉灶，引入新机制呢？

其实仔细想想，上述两种看似不同的思路，其背后目标是一样的：我们需要一个新机制。如果我们选择魔改路线，那首先就得要一个新机制来进行魔改啊！那既然横竖都得新机制，那么还不如把这个机制单独拿出来，作为一个通用的功能，保留原`Channel::enableReading()`不动。

这个新机制是利用Linux上的`eventfd`API 实现的。

由于`EventLoop`线程执行的是一个无限循环，当`EventLoop::poller`监听的fd上无事件发生时，它会一直阻塞在`poll`调用上。那么我们可以把**其他线程要求本IO线程执行用户回调**这一过程作为一个事件，交给 poller 去监听。poller 监听的正是 eventfd。

eventfd的作用可以被视为一个线程间的管道。一个线程可以通过eventfd向另一个线程发送消息。

## Detail
我们在`EventLoop`对象中，添加数据成员`int wakeupfd_`，在其构造函数中利用`createEventfd()`对其初始化，并且构造包含该`wakeupfd_`的channel对象。粗略代码如下：
```c++
// EventLoop.h
class EventLoop{
    ...

    bool callingPendingFunctors_;
    int wakeupfd_;   // 其他线程通过该 eventfd 来唤醒本 IO 线程
    boost::scoped_ptr<Channel> wakeupChannel_;  // 将被其他线程唤醒作为一个事件来监听

    MutexLock mutex_;   // 用来保护 pendingFunctors_
    std::vector<Functor> pendingFunctors_;  // 保存所有用户回调函数
    ...
};

// EventLoop.cpp
EventLoop::EventLoop(): ..., 
                        callingPendingFunctors_(false),
                        wakeupFd_(createEventfd()),
                        wakeupChannel_(new Channel(this, wakeupFd_))
{
    ...
    // 关键的两步，这两个函数只能在 IO 线程中执行
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}
// Thread Safe
void EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } 
  else {
    queueInLoop(cb);    // 如果是其他线程要求本 IO 线程执行回调，则进行分发
  }
}

// 将 cb 入队列
// 向 eventfd 写数据，使得 IO thread 可以立刻从阻塞的 poll 中返回
void EventLoop::queueInLoop(const Functor& cb) {
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();     // 将本线程从阻塞 poll 中唤醒
  }
}

// 将本线程从阻塞 poll 中唤醒
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n
              << " bytes instead of 8 bytes";
  }
}

// eventfd 可读时的处理函数
void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n
              << " bytes instead of 8 bytes";
  }
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
         it != activeChannels_.end(); ++it) {
      (*it)->handleEvent();
    }
    doPendingFunctors();     // 执行来自其他线程的用户回调函数
  }

  LOG_TRACE << "EventLoop " << this << " stio looping";
  looping_ = false;
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }

  callingPendingFunctors_ = false;
}
```
大致原理就是上述代码所体现的。

## 添加 EventLoopThread 类
这个类的作用是对 IO 线程，又称 EventLoopThread 的封装。封装的目的在于在未来更好地实现线程池。具体没啥好分析的，看代码就行了。

## 完善 TimerQueue
muduo0.3 中的TimerQueue不是线程安全的，即：如果我在另一个线程中，执行类似如下代码将会报错：
```c++
#include <stdio.h>

#include "EventLoop.h"
#include "EventLoopThread.h"

void runInThread() {
  printf("runInThread(): pid = %d, tid = %d, time: %s\n", getpid(),
         muduo::CurrentThread::tid(),
         muduo::Timestamp::now().toString().c_str());
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());

  muduo::EventLoopThread loopThread;
  muduo::EventLoop* loop = loopThread.startLoop();

  loop->runInLoop(runInThread);
  printf("main thread called runInLoop(): time: %s\n",
         muduo::Timestamp::now().toString().c_str());
  sleep(1);

  loop->runAfter(2, runInThread);
  printf("main thread called runAfter(): time: %s\n",
         muduo::Timestamp::now().toString().c_str());
  sleep(3);
  loop->quit();

  printf("exit main(): time: %s\n", muduo::Timestamp::now().toString().c_str());
}
```
核心问题在于`TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval)`不是线程安全的。借助之前的`EventLoop::runInLoop()`可以将该函数变为线程安全的。具体实现见代码。
