#ifndef WIN_MAIN_HPP
#define WIN_MAIN_HPP

//#define WIN_USE_SOUND_INTEGRATION_TESTS

// redifined platform macros for conditional compilation
#if defined __linux__
#define WINPLAT_LINUX
#define WIN_USE_OPENGL
#include <iostream>

#elif defined _WIN32
#define WINPLAT_WINDOWS
#define WIN_USE_OPENGL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#else
#error "unsupported platform"
#endif

#include <string>

#define WIN_NO_COPY_MOVE(classname) \
	classname(const classname&) = delete; \
	classname(classname&&) = delete; \
	void operator=(const classname&) = delete; \
	void operator=(classname&&) = delete

namespace win
{

[[noreturn]] inline void bug(const std::string &msg)
{
#ifdef WINPLAT_WINDOWS
	MessageBox(NULL, ("IMPLEMENTATION BUG:\n" + msg).c_str(), "BUG", MB_ICONEXCLAMATION);
#else
	std::cerr << "IMPLEMENTATION BUG:\n=================\n" << msg << "\n=================" << std::endl;
#endif
	std::abort();
}

}

#endif
