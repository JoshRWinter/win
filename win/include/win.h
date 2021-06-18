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

#include "../src/event.h"
#include "../src/AssetRoll.h"
#include "../src/Utility.h"
#include "../src/Texture.h"
#include "../src/Font.h"
#include "../src/Atlas.h"
#include "../src/Sound.h"
#include "../src/objectpool.hpp"
#include "../src/SoundBank.h"
#include "../src/SoundEngine.h"
#include "../src/Display.h"

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
