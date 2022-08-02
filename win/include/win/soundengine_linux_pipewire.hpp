#ifndef WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP
#define WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP

#include <pipewire/pipewire.h>

#include <win/soundengine_linux.hpp>
#include <win/soundcache.hpp>
#include <win/soundmixer.hpp>
#include <win/soundresidencypriority.hpp>

namespace win
{

class SoundEngineLinuxPipeWire : public SoundEngineLinuxProxy
{
public:
	SoundEngineLinuxPipeWire(AssetRoll&);
	virtual ~SoundEngineLinuxPipeWire() override;

	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

	virtual std::uint32_t play(const char*, win::SoundResidencyPriority, float, bool, int) override;
	virtual std::uint32_t play(const char*, win::SoundResidencyPriority, float, float, float, bool, int) override;
	virtual void pause(std::uint32_t) override;
	virtual void resume(std::uint32_t) override;
	virtual void config(std::uint32_t, float, float) override;

private:
	static void stream_process(void*);
	void cleanup(bool);

	// pipewire nonsense
	pw_thread_loop *loop;
	/*
	pw_context *context;
	pw_core *core;
	pw_registry *registry;
	*/
	pw_stream *stream;
	pw_stream_events events;

	SoundMixer mixer;
};

}

#endif
