#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <dlfcn.h>
#include <unistd.h>

#include <win/sound/soundengine.hpp>

#include <win/sound/soundengine_linux_pipewire.hpp>
#include <win/sound/soundengine_linux_pipewire_functions.hpp>

#include <win/sound/soundengine_linux_pulseaudio.hpp>
#include <win/sound/soundengine_linux_pulseaudio_functions.hpp>

#include <win/sound/soundengine_linux_dummy.hpp>

namespace win
{

SoundEngine::SoundEngine(AssetRoll &asset_roll)
{
	const char *libpipewire = "/usr/lib/libpipewire-0.3.so.0";
	const char *libpulse = "/usr/lib/libpulse.so.0";

	if ((so = dlopen(libpipewire, RTLD_LAZY)) != NULL)
	{
		win::load_pipewire_functions(so);
		inner.reset(new SoundEngineLinuxPipeWire(asset_roll));
		fprintf(stderr, "Selecting pipewire for sound\n");
	}
	else if ((so = dlopen(libpulse, RTLD_LAZY)) != NULL)
	{
		win::load_pulseaudio_functions(so);
		inner.reset(new SoundEngineLinuxPulseAudio(asset_roll));
		fprintf(stderr, "Selecting pulseaudio for sound\n");
	}
	else
	{
		fprintf(stderr, "Looking for PipeWire (%s) or PulseAudio (%s) and found neither.", libpipewire, libpulse);
		inner.reset(new SoundEngineLinuxDummy());
	}
}

SoundEngine::~SoundEngine()
{
	dlclose(so);
}

}

#endif
