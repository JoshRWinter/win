#pragma once

#include <win/win.hpp>

#ifdef WINPLAT_WINDOWS

#include <mutex>
#include <thread>
#include <chrono>

#include <mmsystem.h>
#include <dsound.h>

#include <win/AssetRoll.hpp>
#include <win/sound/SoundEngine.hpp>
#include <win/sound/SoundMixer.hpp>

namespace win
{

class SoundEngineWindowsDirectSound : public SoundEngineBase
{
	WIN_NO_COPY_MOVE(SoundEngineWindowsDirectSound);

	constexpr static float write_freq_micro = 4000;
	constexpr static int write_size_stereo_samples = 360;
	constexpr static int buffer_len_bytes = (write_size_stereo_samples * 2 * sizeof(std::int16_t)) * 3;

public:
	explicit SoundEngineWindowsDirectSound(AssetRoll &roll);
	~SoundEngineWindowsDirectSound() override;

	std::uint32_t play(const SoundEnginePlayCommand &cmd) override;
	void save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &configs) override;

private:
	static void loop(SoundEngineWindowsDirectSound &engine, std::atomic<bool> &cancel);
	bool write();

	std::thread loop_thread;
	std::atomic<bool> loop_cancel;
	std::mutex loop_lock;

	IDirectSound8 *context;
	//IDirectSoundBuffer *primary;
	IDirectSoundBuffer8 *secondary;
	DWORD write_position;

	SoundMixer mixer;
};

}

#endif
