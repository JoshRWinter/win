cmake_minimum_required(VERSION 3.0)

if(WIN32)
	add_executable(driver WIN32 driver.cpp)
else()
	add_executable(driver driver.cpp)
endif()
target_link_libraries(driver win)
