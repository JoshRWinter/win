#ifndef WIN_EVENT_H
#define WIN_EVENT_H

namespace win
{

#if defined WINPLAT_LINUX

enum class event
{
	CLOSE,
	NONE
};

#elif defined WINPLAT_WINDOWS

#else
#error "unsupported platform"
#endif

}

#endif
