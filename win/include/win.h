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

#include "../src/event.hpp"
#include "../src/assetroll.hpp"
#include "../src/utility.hpp"
#include "../src/targa.hpp"
#include "../src/font.hpp"
#include "../src/atlas.hpp"
#include "../src/sound.hpp"
#include "../src/objectpool.hpp"
#include "../src/soundbank.hpp"
#include "../src/soundengine.hpp"
#include "../src/display.hpp"

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
