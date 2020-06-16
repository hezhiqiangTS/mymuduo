// TimerQueue 中保存目前尚未到期的所有Timer
#ifndef MUDUO_NET_TIMEQUEUE_H
#define MUDUO_NET_TIMEQUEUE_H
#include <boost/noncopyable.hpp>
#include <set>
#include <vector>

#include "../muduo/base/Mutex.h"
#include "../muduo/base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace muduo {
class EventLoop;
class Timer;
class TimerId;

class TimerQueue : boost::noncopyable {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();
  TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);

 private:
  typedef std::pair<Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerList;

  void addTimerInLoop(Timer* timer);
  void handleRead();

  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);
  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;

  Channel timerfdChannel_;
  // timers_ 按照 Timer.expiration 排序
  TimerList timers_;
};

};  // namespace muduo

#endif