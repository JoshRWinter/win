#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <dlfcn.h>
#include <unistd.h>

#include <win/sound/soundengine.hpp>
#include <win/sound/soundengine_linux_pulseaudio.hpp>
#include <win/sound/soundengine_linux_pipewire.hpp>

namespace win
{

SoundEngine::SoundEngine(AssetRoll &asset_roll)
{
	so = dlopen("/usr/lib/libpulse.so.0", RTLD_LAZY);
	if (so == NULL)
		win::bug("nope");

	winpa::load_pulseaudio_functions(so);
	inner.reset(new SoundEngineLinuxPipeWire(asset_roll));
}

SoundEngine::~SoundEngine()
{
	dlclose(so);
}

}

#endif
