#include <cmath>

#include <win/win.hpp>
#if defined WINPLAT_LINUX
#include <unistd.h>
#elif defined WINPLAT_WINDOWS
#include <windows.h>
#endif

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#include <win/ogg.hpp>
