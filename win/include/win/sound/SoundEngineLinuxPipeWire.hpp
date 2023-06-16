#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <pipewire/pipewire.h>

#include <win/sound/SoundEngine.hpp>
#include <win/sound/SoundMixer.hpp>

namespace win
{

class SoundEngineLinuxPipeWire : public SoundEngineBase
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

public:
	SoundEngineLinuxPipeWire(AssetRoll &roll, const char *soname);
	~SoundEngineLinuxPipeWire() override;

	std::uint32_t play(const SoundEnginePlayCommand &cmd) override;
	void save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &configs) override;

private:
	static void stream_process(void*);
	void load_functions();

	void *so;

	// pipewire nonsense
	pw_thread_loop *loop;
	pw_stream *stream;
	pw_stream_events events;

	SoundMixer mixer;
};

}

#endif
