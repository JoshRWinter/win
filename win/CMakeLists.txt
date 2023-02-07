cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(win STATIC
	src/Atlas.cpp
	src/Stream.cpp
	src/GL.cpp
	src/DisplayLinux.cpp
	src/DisplayWindows.cpp
	src/Font.cpp
	src/FontRenderer.cpp
	src/AssetRoll.cpp
	src/AssetRollStreamRaw.cpp
	src/AssetRollStreamCompressed.cpp
	src/Targa.cpp
	src/Utility.cpp
	src/sound/PitchScaleEffect.cpp
	src/sound/PcmStream.cpp
	src/sound/PcmResource.cpp
	src/sound/PcmDecoder.cpp
	src/sound/SoundCache.cpp
	src/sound/SoundMixer.cpp
	src/sound/SoundEngine.cpp
	src/sound/SoundEngineLinuxPulseAudio.cpp
	src/sound/SoundEngineLinuxPipeWire.cpp
	src/sound/SoundEngineWindowsDirectSound.cpp

	include/win/Win.hpp
	include/win/Pool.hpp
	include/win/RingBuffer.hpp
	include/win/GL.hpp
	include/win/Display.hpp
	include/win/Font.hpp
	include/win/FontRenderer.hpp
	include/win/Stream.hpp
	include/win/AssetRoll.hpp
	include/win/AssetRollStreamRaw.hpp
	include/win/AssetRollStreamCompressed.hpp
	include/win/Atlas.hpp
	include/win/Targa.hpp
	include/win/Utility.hpp
	include/win/Event.hpp
	include/win/sound/FloatPcmProvider.hpp
	include/win/sound/SoundEffect.hpp
	include/win/sound/PitchScaleEffect.hpp
	include/win/sound/PcmStream.hpp
	include/win/sound/PcmResource.hpp
	include/win/sound/PcmDecoder.hpp
	include/win/sound/Sound.hpp
	include/win/sound/SoundCache.hpp
	include/win/sound/SoundMixer.hpp
	include/win/sound/ActiveSoundStore.hpp
	include/win/sound/SoundEngine.hpp
	include/win/sound/SoundEngineWindowsDirectSound.hpp
	include/win/sound/SoundEngineLinuxDummy.hpp
	include/win/sound/SoundEngineLinuxPulseAudio.hpp
	include/win/sound/SoundEngineLinuxPipeWire.hpp
	include/win/sound/test/SoundIntegrationTests.hpp
)

if(WIN32)
	find_package(OpenGL REQUIRED)
	find_package(DSound REQUIRED)

	target_include_directories(win
		PUBLIC
		include
		${OPENGL_INCLUDE_DIRS}
		${DSOUND_INCLUDE_DIRS}
		PRIVATE
		include/freetype
		include/ogg
		include/vorbis
		include/zlib
		glm
		)

	target_link_libraries(win
		PRIVATE
		${OPENGL_LIBRARIES}
		${DSOUND_LIBRARIES}
		dxguid.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/zlib.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/freetype.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/ogg.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/vorbis.lib
		)

	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	set(OpenGL_GL_PREFERENCE LEGACY)

	find_package(Freetype REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(X11 REQUIRED)
	find_package(Threads REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(OggVorbis REQUIRED)
	find_package(PipeWire REQUIRED)
	find_package(PulseAudio REQUIRED)

	target_include_directories(win
		PUBLIC
		include
		PRIVATE
		${FREETYPE_INCLUDE_DIRS}
		${PipeWire_INCLUDE_DIRS}
		${Spa_INCLUDE_DIRS}
		${PULSEAUDIO_INCLUDE_DIRS}
		glm
		)

	target_link_libraries(win
		PRIVATE
		${FREETYPE_LIBRARIES}
		${X11_LIBRARIES}
		${OPENGL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${ZLIB_LIBRARIES}
		${OGGVORBIS_LIBRARIES}
		)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(win PRIVATE -fno-exceptions)
	endif()
endif()

if (USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(win PRIVATE -fsanitize=address)
	target_link_options(win PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(win PRIVATE -fsanitize=undefined)
		target_link_options(win PRIVATE -fsanitize=undefined)
	endif()
endif()
