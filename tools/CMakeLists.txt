cmake_minimum_required(VERSION 3.0)
project(wintools)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../win ${CMAKE_CURRENT_BINARY_DIR}/win)

set(CMAKE_CXX_STANDARD 17)

add_executable(roll roll/roll.cpp)

add_executable(atlas atlas/main.cpp atlas/Targa.cpp atlas/Atlas.cpp)

add_executable(atlasizer atlasizer/main.cpp atlasizer/targa.cpp atlasizer/targa.hpp)
target_link_libraries(atlasizer win)

if(WIN32)
	target_include_directories(roll PRIVATE ../win/include/zlib)
	target_link_libraries(roll PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../win/lib/zlib.lib)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	find_package(ZLIB REQUIRED)

	target_link_libraries(roll ${ZLIB_LIBRARIES})
endif()

