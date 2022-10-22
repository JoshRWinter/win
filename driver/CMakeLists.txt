cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_executable(driver WIN32 driver.cpp)
		add_executable(sounddriver WIN32 sounddriver.cpp)
	else()
		add_executable(driver driver.cpp)
		add_executable(sounddriver sounddriver.cpp)
	endif()

	# /w3 is too aggressive
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	add_executable(driver driver.cpp)
	add_executable(sounddriver sounddriver.cpp)
endif()


target_link_libraries(driver win)
target_link_libraries(sounddriver win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(driver PRIVATE -fno-exceptions)
	target_compile_options(sounddriver PRIVATE -fno-exceptions)

	if(USE_ASAN)
		target_compile_options(driver PRIVATE -fsanitize=address -fsanitize=undefined)
		target_link_options(driver PRIVATE -fsanitize=address -fsanitize=undefined)
		target_compile_options(sounddriver PRIVATE -fsanitize=address -fsanitize=undefined)
		target_link_options(sounddriver PRIVATE -fsanitize=address -fsanitize=undefined)
		message("Enabling ASAN")
	endif()
endif()
