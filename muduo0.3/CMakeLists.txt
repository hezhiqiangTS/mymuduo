cmake_minimum_required(VERSION 3.5)
project(TimerQueueTest)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR})

find_library(muduobase libmuduo_base.a /home/build/release-install-cpp11/lib/)
find_package(Threads)

set(SRC_LIST test4.cpp EventLoop.cpp Channel.cpp Poller.cpp TimerQueue.cpp Timer.cpp)

add_executable(test4 ${SRC_LIST})
target_link_libraries(test4 ${muduobase} ${CMAKE_THREAD_LIBS_INIT})