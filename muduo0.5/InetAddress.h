#ifndef MUDUO_NET_INETADDRESS_H
#define MUDUO_NET_INETADDRESS_H

#include <netinet/in.h>

#include <string>

#include "../muduo/base/copyable.h"

namespace muduo {
// Wrapper of struct sockaddr_in
class InetAddress : public muduo::copyable {
 public:
  explicit InetAddress(uint16_t port);
  InetAddress(const std::string& ip, uint16_t port);
  InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

  std::string toHostPort() const;
  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

 private:
  struct sockaddr_in addr_;
};

};  // namespace muduo

#endif