在muduo0.1的基础上添加了Reactor模式的实现。

三个关键类：EventLoop, Channel, Poller。这三个类完成了对Reactor模式的抽象。

如果不考虑Reactor模式，使用面向过程的方法，基于Poll实现一个TCP Server，伪代码结构如下：
```c
  // 完成对 Listenfd，Server Address 初始化
  ...

  // pollfd 集合初始化
  struct pollfd event_set[INIT_SIZE];
  event_set[0].fd = listen_fd;
  event_set[0].events = POLLIN;

  int i;
  for (i = 1; i < INIT_SIZE; i++) {
    event_set[i].fd = -1;
  }

  // 整个 for 循环就是一个 EventLoop
  for (;;) {
    // 主线程阻塞在 poll 调用
    ready_number = poll(event_set, INIT_SIZE, -1);
    
    // 进行事件的分发
    for (i = 0; i < INIT_SIZE; i++) {
        // 就绪事件是新连接就绪
        if(event_set[i] == listenfd && event_set[i].revent & POLLIN){   
            // 调用处理新连接就绪的处理函数
            handleNewConnection();
            // 将 Connected Fd 加入监听列表
            update(event_set, connfd);
        }

        // Connected Fd 上有新数据到来
        if(event_set[i].event_set[i].revent & POLLIN){
            // 根据就绪事件fd，查找预注册好的事件就绪处理函数
            handler = FindEventCallBack(event_set[i].fd);
            // 调用对应的处理函数
            handler();  
      }
}
```
EventLoop 对象就是对最外部 for 循环的抽象，Poller 对象是对事件分发循环的抽象，而 Channel 对象则是对事件本身的抽象。