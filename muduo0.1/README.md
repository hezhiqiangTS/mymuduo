# EventLoop
每个线程只能包含一个EventLoop对象。
创建了EventLoop对象的线程是IO线程，
其主要作用是执行EventLoop::Loop()进行IO事件的分发。
