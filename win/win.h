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
#include <windows.h>

#else
#error "unsupported platform"
#endif

#include <exception>
#include <string>
#include <iostream>
#include <cstdlib>

namespace win
{

#if defined WINPLAT_WINDOWS
	typedef HWND window_handle;
#elif defined WINPLAT_LINUX
	typedef Window window_handle;
#endif
}

#include "system.h"
#include "resource.h"
#include "event.h"
#include "utility.h"
#include "font.h"
#include "display.h"
#include "audio.h"

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

inline void bug(const std::string &msg)
{
	std::cerr << "IMPLEMENTATION BUG:\n=================\n" << msg << "\n=================" << std::endl;
	std::abort();
}

}

#endif
