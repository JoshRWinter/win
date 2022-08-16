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

	//auto key = se.play("gow.ogg", win::SoundResidencyPriority::high, 1.0f, 1.0f, 1.0f, false, 52 * 60 * 44100 * 2);
	auto key = se.play("gow.ogg", win::SoundResidencyPriority::high, 1.0f, true, (60 + 13 + 0.75) * 60 * 44100 * 2);
	usleep(1000 * 60 * 1000);

	return 0;
}
