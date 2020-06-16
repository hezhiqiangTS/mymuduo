# Acceptor 类
用于管理连接建立事件。
关键数据成员：
* `EventLoop* loop_`
* `Socket acceptSocket_`
* `Channel acceptChannel_`
* `NewConnectionCallback newConnectionCallback_`

`Acceptor::Acceptor(EventLoop*, const InetAddress&)`  用来初始化上述关键数据成员。

其中最重要的是在 `Acceptor` 构造函数中完成对 `Channel` 对象的初始化，`Channel` 对象管理 `Socket.fd()`，`Channel::setReadCallback` 用来为 listenfd 设置连接建立时的callback。

注意，`Acceptor`构造函数为 listenfd 设置的callback并不是实际callback，而是将`Acceptor::newConnectionCallback_` 设置为其callback，`Acceptor::newConnectionCallback_` 指向用户通过 `Acceptor::setNewConnectionCallback` 设置的真正的 Connection Established Callback。

`Acceptor::listen()` 方法完成了`listenfd`的listen动作，并且调用`Chennel.enbaleReading()`，该调用最终会完成将 callback 传递给底层 Poller 或者 Epoller 对象的动作。

# TcpConnection 类

核心类。抽象了一个TcpConnection。
首先，TcpConnection 类描述的是一条已经完成建立的连接。所以需要一个对应 connfd 的接收缓冲区中有数据到达时的 callback，即 messagecallback 成员。

很容易想到，TcpConnection 对象的创建应该在 `Acceptor::newConnectionCallback_` 回调函数的执行中进行。

当 `TcpConnection`对象创建完毕后，我们还需要调用 `TcpConnection.channel_->enablReading()`，来真正将 messagecallback 传递给底层的 POller 或者 Epoller 对象，所以设置了 `TcpConnection::connectEstablished` 方法。该方法需要在 TcpConnection 对象调用过 `TcpConnection.setConnectionCallback` 和 `TcpConnection.setMessageCallback` 调用之后进行。

其中 `TcpConnection::setConnectionCallbcak` 方法从逻辑上来说是不需要的，因为当我们创建好 `TcpConnection` 对象之后，连接建立的动作已经完成，字面上的 `ConnectionCallback` 永远都应该是 `Acceptor` 对象的 conncetionCallback。这里设置之所以 TcpConnection::connectionCallback ，主要还是希望在连接建立过程添加一些用户回调，使得用户可以在连接建立时进行一些定制操作，比如打印调试信息等。`TcpConnection::ConnectionCallback` 将会在 `TcpConnection::connectionEstablished()` 方式中被调用**一次**。

关键的 `TcpConnection::connectEstablished()` 实现如下：
```c++
// 必须要在 TcpConnection.setConnectionCallback() 和 conn->setMessageCallback() 被调用之后调用
void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading(); // 将 messageCallback 注册到 channel 底层的 Dispatcher 中

  connectionCallback_(shared_from_this());
}
```
# TcpServer 类
关键数据成员：
* EventLoop* 
* Acceptor
* std::map<std::string, TcpConnectionPtr>
* ConnectionCallback connectionCallback_;
* MessageCallback messageCallback_;

TcpServer 包含一个 Acceptor，将 Acceptor::newConnectionCallback_ 绑定为 TcpServer::newConnection 

TcpServer::newConnection 中完成对 TcpConnection 对象的创建，同时将 TcpConnection::connectioncallback 和 TcpConnection::messagecallback 设置为用户回调函数的地址，即 TcpServer::connectionCallback_ 和 TcpServer::messageCallback。

一个使用 TcpServer 类的例子：
```c++
void onConnection(const muduo::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const muduo::TcpConnectionPtr& conn, const char* data,
               ssize_t len) {
  printf("onMessage(): received %zd bytes from connection [%s]\n", len,
         conn->name().c_str());
}

int main() {
  printf("main(): pid = %d\n", getpid());

  muduo::InetAddress listenAddr(9981);
  muduo::EventLoop loop;

  muduo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
}
```
