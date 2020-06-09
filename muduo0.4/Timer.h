#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <boost/noncopyable.hpp>

#include "../muduo/base/Timestamp.h"
#include "Callbacks.h"

namespace muduo {

class Timer : boost::noncopyable {
 public:
  Timer(const TimerCallback& cb, Timestamp when, double interval)
      : callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval_ > 0) {}

  // 执行 Timer 对应的 CallBack
  void run() const { callback_(); }

  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }

  void restart(Timestamp now);

 private:
  const TimerCallback callback_;
  // 超时时间
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
};
}  // namespace muduo

#endif