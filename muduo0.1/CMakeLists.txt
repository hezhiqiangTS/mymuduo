cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 11)
message("muduo0.1: ${INCLUDE_DIRECTORIES}")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(test_eventloop test_Eventloop.cpp EventLoop.cpp)

target_link_libraries(test_eventloop gtest muduo_base)

# add_test(test_eventloop ${CMAKE_BINARY_DIR}/test/${bustub_test_name} --gtest_color=yes
#             --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${bustub_test_name}.xml)