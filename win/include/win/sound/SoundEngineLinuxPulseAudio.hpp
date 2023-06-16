#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <pulse/pulseaudio.h>

#include <win/sound/SoundEngine.hpp>
#include <win/sound/SoundMixer.hpp>

namespace win
{

class SoundEngineLinuxPulseAudio : public SoundEngineBase
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxPulseAudio);

public:
	SoundEngineLinuxPulseAudio(AssetRoll &name, const char *soname);
	~SoundEngineLinuxPulseAudio() override;

	std::uint32_t play(const SoundEnginePlayCommand &cmd) override;
	void save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &configs) override;

private:
	static void process(pa_stream *stream, size_t request_bytes, void *userdata);
	void load_functions();

	void *so;

	pa_stream *stream;
	pa_context *context;
	pa_threaded_mainloop *loop;

	SoundMixer mixer;
};

}

#endif
