cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_executable(Driver WIN32 Driver.cpp)
		add_executable(SoundDriver WIN32 SoundDriver.cpp)
	else()
		add_executable(Driver Driver.cpp)
		add_executable(SoundDriver SoundDriver.cpp)
	endif()

	# /w3 is too aggressive
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	add_executable(Driver Driver.cpp)
	add_executable(SoundDriver SoundDriver.cpp)
endif()

target_link_libraries(Driver win)
target_link_libraries(SoundDriver win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(Driver PRIVATE -fno-exceptions)
	target_compile_options(SoundDriver PRIVATE -fno-exceptions)
endif()

if (USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(Driver PRIVATE -fsanitize=address)
	target_link_options(Driver PRIVATE -fsanitize=address)

	target_compile_options(SoundDriver PRIVATE -fsanitize=address)
	target_link_options(SoundDriver PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(Driver PRIVATE -fsanitize=undefined)
		target_link_options(Driver PRIVATE -fsanitize=undefined)

		target_compile_options(SoundDriver PRIVATE -fsanitize=undefined)
		target_link_options(SoundDriver PRIVATE -fsanitize=undefined)
	endif()
endif()
