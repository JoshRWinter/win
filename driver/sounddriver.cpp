#include <win/win.hpp>
#include <win/soundengine.hpp>

#if defined WINPLAT_WINDOWS
#include <windows.h>
static void sleep(int seconds)
{
	Sleep(seconds * 1000);
}
#elif defined WINPLAT_LINUX
#include <unistd.h>
#endif

/*
int main()
{
	win::AssetRoll roll("/home/josh/soundtesting/music.roll");
	win::SoundEngine se(roll);

	auto key = se.play("gow.ogg");
	se.config(key, 1.0, 1.0);

	sleep(10);
	return 0;
}

*/
