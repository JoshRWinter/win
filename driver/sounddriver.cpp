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

int main()
{
	win::AssetRoll roll("/home/josh/soundtesting/music.roll");
	win::SoundEngine se(roll);

	auto key = se.play(win::SoundPriority::high, "gow.ogg", 1.0, 1.0, false);
	//auto key = se.play(win::SoundPriority::high, "../programming/fishtank/assets_local/platform_destroy.ogg");
	se.config(key, 1.0, 0.3);

	sleep(600);
	return 0;
}
