cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 17)

add_library(win STATIC
	src/apack.cpp
	src/atlas.cpp
	src/audio_engine.cpp
	src/display.cpp
	src/font.cpp
	src/joystick.cpp
	src/roll.cpp
	src/system.cpp
	src/tpack.cpp
	src/utility.cpp
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
endif()
