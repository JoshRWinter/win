cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 17)

add_library(win STATIC
	src/Sound.cpp
	src/Atlas.cpp
	src/SoundEngine.cpp
	src/SoundBank.cpp
	src/Display.cpp
	src/Font.cpp
	src/AssetRoll.cpp
	src/Texture.cpp
	src/Utility.cpp

	src/Sound.h
	src/Atlas.h
	src/SoundEngine.h
	src/SoundBank.h
	src/Display.h
	src/Font.h
	src/AssetRoll.h
	src/Texture.h
	src/Utility.h
	src/event.h
	src/objectpool.hpp
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
	find_package(PulseAudio REQUIRED)

	target_include_directories(win
		PUBLIC
		include
		PRIVATE
		${FREETYPE_INCLUDE_DIRS}
		)

	target_link_libraries(win
		PRIVATE
		${FREETYPE_LIBRARIES}
		${X11_LIBRARIES}
		${OPENGL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${ZLIB_LIBRARIES}
		${OGGVORBIS_LIBRARIES}
		${PULSEAUDIO_LIBRARIES}
		)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(win PRIVATE -fno-exceptions)
	endif()
endif()
