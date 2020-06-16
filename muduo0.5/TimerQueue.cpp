#define __STDC_LIMIT_MACROS
#include "TimerQueue.h"

#include <sys/timerfd.h>

#include <boost/bind.hpp>

#include "../muduo/base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

namespace muduo {

namespace detail {
int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

// 计算 now->when 的时间长度
struct timespec howMuchTimeFromNow(Timestamp when) {
  int64_t microseconds =
      when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100) {
    microseconds = 100;
  }

  struct timespec ts;
  ts.tv_sec =
      static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

// Read expiration times since last to now
void readTimerfd(int timerfd, Timestamp now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at "
            << now.toString();
  if (n != sizeof howmany) {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n
              << " bytes instead of 8";
  }
}
//将 timerfd 的 下次 expiration time 设置为 expiration
void resetTimerfd(int timerfd, Timestamp expiration) {
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOG_SYSERR << "timerfd_settime()";
  }
}

}  // namespace detail
}  // namespace muduo

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_() {
  timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::handleRead, this));
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
    delete it->second;
  }
}

// TimerQueue中发生定时器超时的处理函数
void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);

  std::vector<Entry> expired = getExpired(now);

  for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end();
       ++it) {
    it->second->run();
  }

  reset(expired, now);
}

// 在 TimerQueue::TimerList 中添加一个新的 Timer(cb, when, interval)
// 通过 Loop->runInLoop 进行线程间的转发，没有真正执行添加 Timer 的动作
// 真正的添加交给 TimerQueue::addTimerInLoop
TimerId TimerQueue::addTimer(const TimerCallback& cb, Timestamp when,
                             double interval) {
  Timer* timer = new Timer(cb, when, interval);
  loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer) {
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  return expired;
}

//
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
  Timestamp nextExpire;

  for (auto it = expired.begin(); it != expired.end(); ++it) {
    if (it->second->repeat()) {
      it->second->restart(now);
      insert(it->second);
    } else {
      delete it->second;
    }
  }

  if (!timers_.empty()) {
    nextExpire = timers_.begin()->second->expiration();
  }
  if (nextExpire.valid()) {
    resetTimerfd(timerfd_, nextExpire);
  }
}

// 将 Timer 加入 TimerQueue::TimerList
// 如果 Timer.expiration 是所有 Timer 中最近的，返回 true
bool TimerQueue::insert(Timer* timer) {
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }

  std::pair<TimerList::iterator, bool> result =
      timers_.insert(std::make_pair(when, timer));
  assert(result.second);
  return earliestChanged;
}