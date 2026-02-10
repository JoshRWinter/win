cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(win STATIC
	#
	# top dog
	#

	include/win/Win.hpp

	#
	# utility
	#

	include/win/ObjectExchanger.hpp

	include/win/SimStateExchanger.hpp

	include/win/Utility.hpp
	src/Utility.cpp

	include/win/Pool.hpp

	include/win/ConcurrentRingBuffer.hpp

	include/win/MappedRingBuffer.hpp

	include/win/SpatialIndex.hpp

	include/win/Atlas.hpp
	src/Atlas.cpp

	include/win/Targa.hpp
	src/Targa.cpp

	include/win/Math.hpp

	include/win/UdpServer.hpp
	src/UdpServer.cpp

	include/win/UdpClient.hpp
	src/UdpClient.cpp

	include/win/WsaSingleton.hpp
	src/WsaSingleton.cpp

	#
	# gl utility
	#

	include/win/gl/GL.hpp
	src/gl/GL.cpp

	include/win/gl/GLAtlas.hpp
	src/gl/GLAtlas.cpp

	include/win/gl/GLFont.hpp
	src/gl/GLFont.cpp

	include/win/gl/GLTextRenderer.hpp
	src/gl/GLTextRenderer.cpp

	include/win/gl/GLMappedRingBuffer.hpp

	#
	# core
	#

	include/win/Event.hpp

	include/win/Stream.hpp

	include/win/AssetRoll.hpp
	src/AssetRoll.cpp

	include/win/MemoryStream.hpp
	src/MemoryStream.cpp

	include/win/FileStream.hpp
	src/FileStream.cpp

	include/win/X11MonitorEnumerator.hpp
	src/X11MonitorEnumerator.cpp

	include/win/Win32MonitorEnumerator.hpp
	src/Win32MonitorEnumerator.cpp

	include/win/MonitorEnumerator.hpp
	src/MonitorEnumerator.cpp

	include/win/DisplayBase.hpp
	src/DisplayBase.cpp

	include/win/X11Display.hpp
	src/X11Display.cpp

	include/win/Win32Display.hpp
	src/Win32Display.cpp

	include/win/Display.hpp

	include/win/Font.hpp
	src/Font.cpp

	include/win/TextRenderer.hpp
	src/TextRenderer.cpp

	#
	# sound core
	#

	include/win/sound/ActiveSoundStore.hpp

	include/win/sound/PcmSource.hpp

	include/win/sound/DecodingPcmSource.hpp
	src/sound/DecodingPcmSource.cpp

	include/win/sound/CachingPcmSource.hpp
	src/sound/CachingPcmSource.cpp

	include/win/sound/CachedPcmSource.hpp
	src/sound/CachedPcmSource.cpp

	include/win/sound/Sound.hpp

	include/win/sound/SoundRepo.hpp
	src/sound/SoundRepo.cpp

	include/win/sound/SoundMixer.hpp
	src/sound/SoundMixer.cpp

	include/win/sound/SoundEngine.hpp
	src/sound/SoundEngine.cpp

	include/win/sound/SoundEngineWindowsDirectSound.hpp
	src/sound/SoundEngineWindowsDirectSound.cpp

	include/win/sound/SoundEngineLinuxDummy.hpp

	include/win/sound/SoundEngineLinuxPulseAudio.hpp
	src/sound/SoundEngineLinuxPulseAudio.cpp

	include/win/sound/SoundEngineLinuxPipeWire.hpp
	src/sound/SoundEngineLinuxPipeWire.cpp
)

if(WIN32)
	find_package(OpenGL REQUIRED)
	find_package(DSound REQUIRED)

	target_include_directories(win
		PUBLIC
		include
		${OPENGL_INCLUDE_DIRS}
		include/GL
		${DSOUND_INCLUDE_DIRS}
		PRIVATE
		include/freetype
		include/ogg
		include/vorbis
		include/zlib
		)

	target_link_libraries(win
		PRIVATE
		${OPENGL_LIBRARIES}
		${DSOUND_LIBRARIES}
		dxgi.lib
		dxguid.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/zlib.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/freetype.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/ogg.lib
		${CMAKE_CURRENT_LIST_DIR}/lib/vorbis.lib
		ws2_32
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
		)

	target_link_libraries(win
		PRIVATE
		${FREETYPE_LIBRARIES}
		${X11_LIBRARIES}
		${OPENGL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${ZLIB_LIBRARIES}
		${OGGVORBIS_LIBRARIES}
		Xrandr
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
