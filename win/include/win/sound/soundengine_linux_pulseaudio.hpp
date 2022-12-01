#ifndef WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_HPP
#define WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pulse/pulseaudio.h>

#include <win/sound/soundengine_linux.hpp>
#include <win/sound/soundmixer.hpp>

namespace win
{

class SoundEngineLinuxPulseAudio : public SoundEngineLinuxProxy
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxPulseAudio);

protected:
	~SoundEngineLinuxPulseAudio();

public:
	SoundEngineLinuxPulseAudio(AssetRoll&);

	std::uint32_t play(const char*, int, float, bool, int) override;
	std::uint32_t play(const char*, int, float, float, float, bool, int) override;
	void apply_effect(std::uint32_t, SoundEffect*) override;
	void remove_effect(std::uint32_t, SoundEffect*) override;
	void pause(std::uint32_t) override;
	void resume(std::uint32_t) override;
	void stop(std::uint32_t) override;
	void config(std::uint32_t, float, float) override;

private:
	static void process(pa_stream*, size_t, void*);

	pa_stream *stream;
	pa_context *context;
	pa_threaded_mainloop *loop;

	SoundMixer mixer;
};

}

#endif

#endif
