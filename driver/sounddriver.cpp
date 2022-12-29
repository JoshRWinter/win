#include <win/win.hpp>
#include <win/sound/soundengine.hpp>
#include <win/sound/pitchscaleeffect.hpp>

#if defined WINPLAT_WINDOWS
#include <windows.h>
static void sleep(int seconds)
{
	Sleep(seconds * 1000);
}
#elif defined WINPLAT_LINUX
#include <unistd.h>
#endif

#define private public
#include <win/display.hpp>

int main()
{
	//win::AssetRoll roll("/home/josh/soundtesting/music.roll");
	win::DisplayOptions opts;
	opts.width = 300;
	opts.height = 100;
	opts.gl_major = 3;
	opts.gl_minor = 3;
	win::Display d(opts);
	win::AssetRoll roll("c:\\users\\josh\\desktop\\music.roll");
	win::SoundEngine se(d, roll);

	//win::PitchScaleEffect effect(1, 2.0f);
	//auto key = se.play("gow.ogg", win::SoundResidencyPriority::high, 1.0f, 1.0f, 1.0f, true, (24 + 0.85) * 60 * 44100 * 2);
	auto key = se.play("gow.ogg", 5, 1.0f, 1.0f, 1.0f, true);
	//se.apply_effect(key, &effect);
	//auto key2 = se.play("soft.ogg", win::SoundResidencyPriority::high, 1.0f, 1.0f, 1.0f, true, 0);
	//auto key3 = se.play("soft.ogg", win::SoundResidencyPriority::high, 1.0f, 1.0f, 1.0f, true, 0);
	//usleep(1'000'000 * 1000);
	bool quit = false;
	d.register_window_handler([&quit](win::WindowEvent e){ quit = true; });
	while (!quit) d.process();
	//Sleep(1000 * 60);
	//se.remove_effect(key, &effect);

	return 0;
}
