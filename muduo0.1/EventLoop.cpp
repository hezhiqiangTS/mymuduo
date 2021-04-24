//
// Created by 何智强 on 2021/4/23.
//

#include "EventLoop.h"
#include "muduo/base/Logging.h"

using namespace muduo;

__thread EventLoop *t_loopInThisThread = nullptr;

EventLoop::EventLoop() : looping_(false), threadId_(CurrentThread::tid()) {
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if(t_loopInThisThread){
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
        << " exists in this thread " << threadId_;
    } else{
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    ::poll(NULL, 0, 5*1000);

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}