cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
message("muduo0.2: ${INCLUDE_DIRECTORIES}")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SRC_LIST test3.cpp EventLoop.cpp Channel.cpp Poller.cpp)

add_executable(test3 ${SRC_LIST})
target_link_libraries(test3 muduo_base)