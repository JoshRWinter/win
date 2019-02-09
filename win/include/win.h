#ifndef WIN_MAIN_H
#define WIN_MAIN_H

// redifined platform macros for conditional compilation
#if defined __linux__
#define WINPLAT_LINUX
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>

#elif defined _WIN32
#define WINPLAT_WINDOWS
#define NOMINMAX
#include <windows.h>

#else
#error "unsupported platform"
#endif

#include <exception>
#include <iostream>
#include <cstdlib>

namespace win
{

#if defined WINPLAT_WINDOWS
	typedef HWND window_handle;
#elif defined WINPLAT_LINUX
	typedef Window window_handle;
#else
#error "unsupported platform"
#endif
}

#include "../src/system.h"
#include "../src/event.h"
#include "../src/utility.h"
#include "../src/roll.h"
#include "../src/font.h"
#include "../src/atlas.h"
#include "../src/tpack.h"
#include "../src/apack.h"
#include "../src/audio_engine.h"
#include "../src/joystick.h"
#include "../src/display.h"

namespace win
{

class exception : public std::exception
{
public:
	exception(const std::string &msg) : msg_(msg) {}
	exception(const char *msg) : msg_(msg) {}

	virtual const char *what() const noexcept { return msg_.c_str(); }

private:
	const std::string msg_;
};

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
