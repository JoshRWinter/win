#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <pulse/pulseaudio.h>

#include <win/sound/SoundEngine.hpp>
#include <win/sound/SoundMixer.hpp>

namespace win
{

class SoundEngineLinuxPulseAudio : public SoundEngineImplementation
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxPulseAudio);

public:
	SoundEngineLinuxPulseAudio(AssetRoll&, const char*);
	~SoundEngineLinuxPulseAudio();

	std::uint32_t play(const SoundEnginePlayCommand&) override;
	void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) override;

private:
	static void process(pa_stream*, size_t, void*);
	void load_functions();

	void *so;

	pa_stream *stream;
	pa_context *context;
	pa_threaded_mainloop *loop;

	SoundMixer mixer;
};

}

#endif
