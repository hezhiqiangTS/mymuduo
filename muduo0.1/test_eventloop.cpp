#include "gtest/gtest.h"
#include "EventLoop.h"
#include "Thread.h"

void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n",
           getpid(), muduo::CurrentThread::tid());

    muduo::EventLoop loop;
    loop.loop();
}


namespace muduo
{

    TEST(EventLoopTests, SimpleTest)
    {
        printf("main(): pid = %d, tid = %d\n",
               getpid(), muduo::CurrentThread::tid());
               muduo::EventLoop loop;
               muduo::Thread thread(threadFunc);
               thread.start();

               loop.loop();
               
    }

    TEST(EventLoopTests, SimpleTest2){
        muduo::EventLoop* g_loop;
        void threadFunc(){
            g_loop->loop();
        }

        muduo::EventLoop loop;
        g_loop = &loop;
        muduo::Thread t(threadFunc);
        t.start();
        t.join();
    }

} // namespace mymuduo