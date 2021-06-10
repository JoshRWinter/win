cmake_minimum_required(VERSION 3.0)

if(NOT WIN32)
	add_custom_command(
		OUTPUT
		${CMAKE_CURRENT_LIST_DIR}/assets.roll
		COMMAND
		cd ${CMAKE_CURRENT_LIST_DIR} && roll assets.roll `cat roll.assets`
		)
endif()

if(WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_executable(driver WIN32 driver.cpp assets.roll)
	else()
		add_executable(driver driver.cpp assets.roll)
	endif()

	# /w3 is too aggressive
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	add_executable(driver driver.cpp assets.roll)
endif()

target_link_libraries(driver win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(driver PRIVATE -fno-exceptions)
endif()
