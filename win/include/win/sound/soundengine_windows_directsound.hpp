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
	constexpr static int buffer_len_bytes = (write_size_stereo_samples * 2 * sizeof(std::int16_t)) * 3;

public:
	SoundEngineWindowsDirectSound(HWND, AssetRoll &roll);
	~SoundEngineWindowsDirectSound();

	std::uint32_t play(const SoundEnginePlayCommand&) override;
	void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) override;

private:
	static void loop(SoundEngineWindowsDirectSound&, std::atomic<bool>&);
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

#endif
