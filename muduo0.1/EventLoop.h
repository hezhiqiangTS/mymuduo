//
// Created by 何智强 on 2021/4/23.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/CurrentThread.h"

namespace muduo{
    class EventLoop : muduo::noncopyable{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void assertInLoopThread(){

    }

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}

private:
    void abortNotInLoopThread();

    bool looping_;
    const pid_t threadId_;
};
};





#endif //MYMUDUO_EVENTLOOP_H
