cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PoolTests PoolTests.cpp)
add_executable(ActiveSoundStoreTests ActiveSoundStoreTests.cpp)
add_executable(RingBufferTests RingBufferTests.cpp)
add_executable(SoundIntegrationTests SoundIntegrationTests.cpp ../win/include/win/sound/test/SoundIntegrationTests.hpp)

target_link_libraries(PoolTests win)
target_link_libraries(ActiveSoundStoreTests win)
target_link_libraries(RingBufferTests win)
target_link_libraries(SoundIntegrationTests win)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(PoolTests PRIVATE -fno-exceptions)
	target_compile_options(ActiveSoundStoreTests PRIVATE -fno-exceptions)
	target_compile_options(RingBufferTests PRIVATE -fno-exceptions)
	target_compile_options(SoundIntegrationTests PRIVATE -fno-exceptions)
endif()

if(USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(PoolTests PRIVATE -fsanitize=address)
	target_link_options(PoolTests PRIVATE -fsanitize=address)

	target_compile_options(ActiveSoundStoreTests PRIVATE -fsanitize=address)
	target_link_options(ActiveSoundStoreTests PRIVATE -fsanitize=address)

	target_compile_options(RingBufferTests PRIVATE -fsanitize=address)
	target_link_options(RingBufferTests PRIVATE -fsanitize=address)

	target_compile_options(SoundIntegrationTests PRIVATE -fsanitize=address)
	target_link_options(SoundIntegrationTests PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(PoolTests PRIVATE -fsanitize=undefined)
		target_link_options(PoolTests PRIVATE -fsanitize=undefined)

		target_compile_options(ActiveSoundStoreTests PRIVATE -fsanitize=undefined)
		target_link_options(ActiveSoundStoreTests PRIVATE -fsanitize=undefined)

		target_compile_options(RingBufferTests PRIVATE -fsanitize=undefined)
		target_link_options(RingBufferTests PRIVATE -fsanitize=undefined)

		target_compile_options(SoundIntegrationTests PRIVATE -fsanitize=undefined)
		target_link_options(SoundIntegrationTests PRIVATE -fsanitize=undefined)
	endif()
endif()
