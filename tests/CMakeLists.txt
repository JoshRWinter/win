cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pool_tests pooltests.cpp)

target_link_libraries(pool_tests win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(pool_tests PRIVATE -fno-exceptions)
endif()
