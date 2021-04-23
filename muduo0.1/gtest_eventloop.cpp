#include "gtest/gtest.h"
#include "EventLoop.h"

void threadFunc(){
    printf("threadFunc(): pid = %d, tid = %d\n",
    getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;
    loop.loop();
}


namespace muduo{

    TEST(EventLoopTests, SimpleTest){

    }

} // namespace mymuduo