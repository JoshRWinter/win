#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS
#include <winsock2.h>
#endif

namespace win
{

#ifdef WINPLAT_WINDOWS
struct WinWsaInit
{
	WinWsaInit()
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(1, 1), &wsa);
	}

	~WinWsaInit()
	{
		WSACleanup();
	}
};
#endif

void initialize_wsa();

}
