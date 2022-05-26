#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <dlfcn.h>
#include <unistd.h>

#include <win/soundengine.hpp>
#include <win/soundengine_linux_pulseaudio.hpp>
#include <win/soundengine_linux_pipewire.hpp>

namespace win
{

SoundEngine::SoundEngine(/*Display &p, */AssetRoll &asset_roll, SoundConfigFn fn)
{
	so = dlopen("/usr/lib/libpulse.so.0", RTLD_LAZY);
	if (so == NULL)
		win::bug("nope");

	winpa::load_pulseaudio_functions(so);
	inner.reset(new SoundEngineLinuxPipeWire(/*p, */asset_roll, fn));
}

SoundEngine::~SoundEngine()
{
	dlclose(so);
}

}

#endif
