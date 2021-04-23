#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>

#include "muduo/base/Timestamp.h"
#include "EventLoop.h"

struct pollfd;
namespace muduo {
class Channel;

class Poller : boost::noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  void updateChannel(Channel* channel);

  void assertInLoopThread() { owerLoop_->assertInLoopThread(); }

 private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;

  EventLoop* owerLoop_;
  PollFdList pollfds_;
  ChannelMap channels_;
};

}  // namespace muduo

#endif