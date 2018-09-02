cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(roll roll.cpp)

if(WIN32)
	target_include_directories(roll PRIVATE ../win/include/zlib)
	target_link_libraries(roll PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../win/lib/zlib.lib)
else()
	find_package(ZLIB REQUIRED)

	target_link_libraries(roll ${ZLIB_LIBRARIES})
endif()
