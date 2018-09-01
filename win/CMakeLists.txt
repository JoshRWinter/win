cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/CMakeModules)
set(CMAKE_CXX_STANDARD 17)

add_library(win STATIC
	src/audio_engine.cpp
	src/display.cpp
	src/font.cpp
	src/joystick.cpp
	src/roll.cpp
	src/system.cpp
	src/utility.cpp
)

if(win32)
	target_include_directories(win
		PUBLIC
			win
			include/GL
		PRIVATE
			include/freetype
			include/ogg
			include/vorbis
			include/zlib
	)
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
