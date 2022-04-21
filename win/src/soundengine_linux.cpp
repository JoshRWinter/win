#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <unistd.h>

#include <win/soundengine.hpp>
#include <win/soundengine_linux_pulseaudio.hpp>

namespace win
{

SoundEngine::SoundEngine(Display &p, AssetRoll &asset_roll, SoundConfigFn fn)
	: inner(new SoundEngineLinuxPulseAudio(p, asset_roll, fn))
{
}

}

#endif
