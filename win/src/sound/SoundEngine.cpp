#include <filesystem>

#include <win/sound/SoundEngine.hpp>

#if defined WINPLAT_LINUX
#include <win/sound/SoundEngineLinuxPipeWire.hpp>
#include <win/sound/SoundEngineLinuxPulseAudio.hpp>
#include <win/sound/SoundEngineLinuxDummy.hpp>
#elif defined WINPLAT_WINDOWS
#include <win/sound/SoundEngineWindowsDirectSound.hpp>
#endif

namespace win
{

SoundEngine::SoundEngine(AssetRoll &roll)
{
#if defined WINPLAT_LINUX
	const char *libpipewire = "/usr/lib/libpipewire-0.3.so.0";
	const char *libpulse = "/usr/lib/libpulse.so.0";

	const char *const pulse_override = "WIN_USE_PULSE";
	const char *const value = getenv(pulse_override);

	if (std::filesystem::exists(libpipewire) && value == NULL)
	{
		fprintf(stderr, "Selecting PipeWire (%s) for sound\n", libpipewire);
		inner.reset(new SoundEngineLinuxPipeWire(roll, libpipewire));
	}
	else if (std::filesystem::exists(libpulse))
	{
		fprintf(stderr, "Selecting PulseAudio (%s) for sound\n", libpulse);
		inner.reset(new SoundEngineLinuxPulseAudio(roll, libpulse));
	}
	else
	{
		fprintf(stderr, "Looking for PipeWire (%s) or PulseAudio (%s) and found neither.\n", libpipewire, libpulse);
		inner.reset(new SoundEngineLinuxDummy());
	}
#elif defined WINPLAT_WINDOWS
	inner.reset(new SoundEngineWindowsDirectSound(roll));
#endif
}

std::uint32_t SoundEngine::play(const char *name, int residency_priority, float compression_priority, float left, float right, bool looping, bool cache, int seek)
{
	SoundEnginePlayCommand cmd(name, residency_priority, compression_priority, left, right, looping, cache, seek);
	return inner->play(cmd);
}

void SoundEngine::pause(std::uint32_t key)
{
	playback_commands.emplace_back(key, false, false);
}

void SoundEngine::resume(std::uint32_t key)
{
	playback_commands.emplace_back(key, true, false);
}

void SoundEngine::stop(std::uint32_t key)
{
	playback_commands.emplace_back(key, false, true);
}

void SoundEngine::config(std::uint32_t key, float left, float right)
{
	config_commands.emplace_back(key, left, right);
}

void SoundEngine::save()
{
	inner->save(playback_commands, config_commands);

	playback_commands.clear();
	config_commands.clear();
}

}
