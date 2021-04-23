cmake_minimum_required(VERSION 3.17)

add_executable(test_eventloop EXCLUDE_FROM_ALL gtest_eventloop.cpp)

target_link_libraries(test_eventloop gtest gmock_main)

add_test(test_eventloop ${CMAKE_BINARY_DIR}/test/${bustub_test_name} --gtest_color=yes
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${bustub_test_name}.xml)