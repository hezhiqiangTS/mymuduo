cmake_minimum_required(VERSION 3.5)
project(TimerQueueTest)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})

find_library(muduobase libmuduo_base.a /home/build/release-install-cpp11/lib/)
find_package(Threads)

set(TEST7_SRC_LIST test7.cpp EventLoop.cpp Channel.cpp Poller.cpp TimerQueue.cpp Timer.cpp EventLoopThread.cpp Acceptor.cpp
    InetAddress.cpp SocketsOps.cpp Socket.cpp)

add_executable(test7 ${TEST7_SRC_LIST})
target_link_libraries(test7 ${muduobase} ${CMAKE_THREAD_LIBS_INIT})