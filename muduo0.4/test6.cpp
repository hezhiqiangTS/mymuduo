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