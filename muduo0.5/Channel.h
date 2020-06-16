#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo {
class EventLoop;

class Channel : boost::noncopyable {
 public:
  typedef boost::function<void()> EventCallback;
  Channel(EventLoop* loop, int fd);
  void handleEvent();
  void setReadCallback(const EventCallback& cb) { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
  void serErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  // Call Channel::update()
  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  // void enableWriting() { events_ |= kWriteEvent; update(); }
  // void disableWriting() { events_ &= ~kWriteEvent; update(); }
  // void disableAll() { events_ = kNoneEvent; update(); }

  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

 private:
  void update();

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;   // 关心的事件
  int revents_;  // 目前就绪的事件
  int index_;    // used by poller
                 // 记录 Channel 在 Poller::ChannelList 中的位置

  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
};
};  // namespace muduo
#endif