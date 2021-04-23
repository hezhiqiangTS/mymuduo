cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(mymuduo
        VERSION 2021.4
        DESCRIPTION "Learn Multiple Threads Programming On Linux"
        LANGUAGES C CXX
        )

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)
message(STATUS "PATH_TO_CMAKELISTS_TXT: ${PATH_TO_CMAKELISTS_TXT}")

if (EXISTS "${PATH_TO_CMAKELISTS_TXT}")
        message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
        Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif()

# https://crascit.com/2015/07/25/cmake-gtest/
# Download and unpack googletest at configure time
# First, Copy configure file to a relative path of binary dir
configure_file("${PROJECT_SOURCE_DIR}/build_support/gtest_CMakeLists.txt.in" googletest-download/CMakeLists.txt)
# Then, Create a sub-process to execute command with specified args
# CMAKE_COMMAND: The full path to cmake executable
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
# Don't override our compiler/linker options when building gtest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  
# Add gtest/gmock header file to 
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

configure_file("${PROJECT_SOURCE_DIR}/build_support/boost_CMakeLists.txt.in" boost-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/boost-download")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/boost-download")