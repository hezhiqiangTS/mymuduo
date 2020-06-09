#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include "../muduo/base/copyable.h"

namespace muduo {
class Timer;

class TimerId : public muduo::copyable {
 public:
  explicit TimerId(Timer* timer) : value_(timer) {}

 private:
  Timer* value_;
};

}  // namespace muduo

#endif