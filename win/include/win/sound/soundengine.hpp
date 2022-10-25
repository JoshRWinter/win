#ifndef WIN_SOUND_ENGINE_HPP
#define WIN_SOUND_ENGINE_HPP

#include <list>
#include <memory>

#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <win/sound/soundengine_linux.hpp>
#elif defined WINPLAT_WINDOWS
#include <win/soundengine_windows.hpp>
#endif

#endif
