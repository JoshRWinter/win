cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pool_tests pooltests.cpp)
add_executable(activesoundstore_tests activesoundstoretests.cpp)
add_executable(ringbuffer_tests ringbuffertests.cpp)
add_executable(soundintegration_tests soundintegration_tests.cpp ../win/include/win/sound/test/soundintegrationtests.hpp)

target_link_libraries(pool_tests win)
target_link_libraries(activesoundstore_tests win)
target_link_libraries(ringbuffer_tests win)
target_link_libraries(soundintegration_tests win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(pool_tests PRIVATE -fno-exceptions)
	target_compile_options(activesoundstore_tests PRIVATE -fno-exceptions)
	target_compile_options(ringbuffer_tests PRIVATE -fno-exceptions)
	target_compile_options(soundintegration_tests PRIVATE -fno-exceptions)
endif()

if(USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(pool_tests PRIVATE -fsanitize=address)
	target_link_options(pool_tests PRIVATE -fsanitize=address)

	target_compile_options(activesoundstore_tests PRIVATE -fsanitize=address)
	target_link_options(activesoundstore_tests PRIVATE -fsanitize=address)

	target_compile_options(ringbuffer_tests PRIVATE -fsanitize=address)
	target_link_options(ringbuffer_tests PRIVATE -fsanitize=address)

	target_compile_options(soundintegration_tests PRIVATE -fsanitize=address)
	target_link_options(soundintegration_tests PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(pool_tests PRIVATE -fsanitize=undefined)
		target_link_options(pool_tests PRIVATE -fsanitize=undefined)

		target_compile_options(activesoundstore_tests PRIVATE -fsanitize=undefined)
		target_link_options(activesoundstore_tests PRIVATE -fsanitize=undefined)

		target_compile_options(ringbuffer_tests PRIVATE -fsanitize=undefined)
		target_link_options(ringbuffer_tests PRIVATE -fsanitize=undefined)

		target_compile_options(soundintegration_tests PRIVATE -fsanitize=undefined)
		target_link_options(soundintegration_tests PRIVATE -fsanitize=undefined)
	endif()
endif()
