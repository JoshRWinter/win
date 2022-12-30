cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(win STATIC
	src/atlas.cpp
	src/stream.cpp
	src/gl.cpp
	src/display_linux.cpp
	src/display_windows.cpp
	src/font.cpp
	src/fontrenderer.cpp
	src/assetroll.cpp
	src/assetrollstreamraw.cpp
	src/assetrollstreamcompressed.cpp
	src/targa.cpp
	src/utility.cpp
	src/sound/pitchscaleeffect.cpp
	src/sound/pcmstream.cpp
	src/sound/pcmresource.cpp
	src/sound/pcmdecoder.cpp
	src/sound/soundcache.cpp
	src/sound/soundmixer.cpp
	src/sound/soundengine.cpp
	src/sound/soundengine_linux_pulseaudio.cpp
	src/sound/soundengine_linux_pipewire.cpp
	src/sound/soundengine_windows_directsound.cpp

	include/win/win.hpp
	include/win/pool.hpp
	include/win/ringbuffer.hpp
	include/win/gl.hpp
	include/win/display.hpp
	include/win/font.hpp
	include/win/fontrenderer.hpp
	include/win/stream.hpp
	include/win/assetroll.hpp
	include/win/assetrollstreamraw.hpp
	include/win/assetrollstreamcompressed.hpp
	include/win/atlas.hpp
	include/win/targa.hpp
	include/win/utility.hpp
	include/win/event.hpp
	include/win/sound/floatpcmprovider.hpp
	include/win/sound/soundeffect.hpp
	include/win/sound/pitchscaleeffect.hpp
	include/win/sound/pcmstream.hpp
	include/win/sound/pcmresource.hpp
	include/win/sound/pcmdecoder.hpp
	include/win/sound/sound.hpp
	include/win/sound/soundcache.hpp
	include/win/sound/soundmixer.hpp
	include/win/sound/activesoundstore.hpp
	include/win/sound/soundengine.hpp
	include/win/sound/soundengine_windows_directsound.hpp
	include/win/sound/soundengine_linux_dummy.hpp
	include/win/sound/soundengine_linux_pulseaudio.hpp
	include/win/sound/soundengine_linux_pipewire.hpp
	include/win/sound/test/soundintegrationtests.hpp
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
