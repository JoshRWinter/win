#ifndef WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP
#define WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP

#include <pipewire/pipewire.h>

#include <win/sound/soundengine_linux.hpp>
#include <win/sound/soundcache.hpp>
#include <win/sound/soundmixer.hpp>
#include <win/sound/soundresidencypriority.hpp>

namespace win
{

class SoundEngineLinuxPipeWire : public SoundEngineLinuxProxy
{
public:
	SoundEngineLinuxPipeWire(AssetRoll&);
	~SoundEngineLinuxPipeWire();

	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

	std::uint32_t play(const char*, win::SoundResidencyPriority, float, bool, int) override;
	std::uint32_t play(const char*, win::SoundResidencyPriority, float, float, float, bool, int) override;
	void apply_effect(std::uint32_t, SoundEffect*) override;
	void remove_effect(std::uint32_t, SoundEffect*) override;
	void pause(std::uint32_t) override;
	void resume(std::uint32_t) override;
	void stop(std::uint32_t) override;
	void config(std::uint32_t, float, float) override;

private:
	static void stream_process(void*);
	void cleanup(bool);

	// pipewire nonsense
	pw_thread_loop *loop;
	pw_context *context;
	pw_core *core;
	pw_registry *registry;
	pw_stream *stream;
	pw_stream_events events;

	SoundMixer mixer;
};

}

#endif
