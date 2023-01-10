#pragma once

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pipewire/pipewire.h>

#include <win/sound/soundengine.hpp>
#include <win/sound/soundmixer.hpp>

namespace win
{

class SoundEngineLinuxPipeWire : public SoundEngineImplementation
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

public:
	SoundEngineLinuxPipeWire(AssetRoll&, const char*);
	~SoundEngineLinuxPipeWire();

	std::uint32_t play(const SoundEnginePlayCommand&) override;
	void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) override;

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
