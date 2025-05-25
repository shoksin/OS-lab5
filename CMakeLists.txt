cmake_minimum_required(VERSION 3.14)
project(NamedPipeFileAccess)

set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

# Fetch GoogleTest via Git
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# Disable install step for gtest
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# On MSVC use shared CRT
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

include_directories(include)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

# Tests
enable_testing()
add_executable(test_protocol tests/test_protocol.cpp)
target_link_libraries(test_protocol gtest gtest_main)
add_test(NAME ProtocolTest COMMAND test_protocol)