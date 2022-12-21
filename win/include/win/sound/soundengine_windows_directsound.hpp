#ifndef WIN_SOUNDENGINE_WINDOWS_DIRECTSOUND_HPP
#define WIN_SOUNDENGINE_WINDOWS_DIRECTSOUND_HPP

#include <win/win.hpp>

#ifdef WINPLAT_WINDOWS

#include <mutex>
#include <thread>
#include <chrono>

#include <mmsystem.h>
#include <dsound.h>

#include <win/assetroll.hpp>
#include <win/sound/soundengine.hpp>
#include <win/sound/soundmixer.hpp>

namespace win
{

class SoundEngineWindowsDirectSound : public SoundEngineImplementation
{
	WIN_NO_COPY_MOVE(SoundEngineWindowsDirectSound);

	constexpr static float write_freq_micro = 4000;
	constexpr static int write_size_stereo_samples = 360;

public:
	SoundEngineWindowsDirectSound(AssetRoll &roll);
	~SoundEngineWindowsDirectSound();

	std::uint32_t play(const SoundEnginePlayCommand&) override;
	void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) override;

private:
	static void loop(SoundEngineWindowsDirectSound&);
	void write();

	std::thread loop_thread;
	std::mutex loop_lock;

	std::chrono::time_point<std::chrono::high_resolution_clock> loop_last_write;

	IDirectSound8 *context;
	IDirectSoundBuffer *primary;
	IDirectSoundBuffer8 *secondary;

	SoundMixer mixer;
};

}

#endif

#endif
