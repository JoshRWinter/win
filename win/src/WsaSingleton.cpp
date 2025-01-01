#include <win/WsaSingleton.hpp>

namespace win
{

void initialize_wsa()
{
#ifdef WINPLAT_WINDOWS
	static WinWsaInit global;
#endif
}

}
