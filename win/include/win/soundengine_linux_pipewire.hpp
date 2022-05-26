#ifndef WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP
#define WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP

#include <pipewire/pipewire.h>

#include <win/activesoundstore.hpp>
#include <win/soundbank.hpp>
#include <win/soundengine_linux.hpp>

namespace win
{

struct PipeWireActiveSound
{
	WIN_NO_COPY_MOVE(PipeWireActiveSound);

	PipeWireActiveSound(Sound &sound)
		: sound(&sound)
		, stream(NULL)
		, flushing(false)
		, done(false)
	{}

	Sound *sound;
	pw_stream *stream;
	bool flushing;
	bool done;
};

class SoundEngineLinuxPipeWire : public SoundEngineLinuxProxy
{
public:
	SoundEngineLinuxPipeWire(/*Display&,*/ AssetRoll&, SoundConfigFn);
	virtual ~SoundEngineLinuxPipeWire() override;

	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

	virtual void process() override;

	virtual std::uint32_t play(const char*, bool = false) override;
	virtual std::uint32_t play(const char*, float, float, bool = false) override;
	virtual std::uint32_t play(const char*, float, float, bool, bool) override;
	virtual void pause(std::uint32_t) override;
	virtual void resume(std::uint32_t) override;
	virtual void stop(std::uint32_t) override;
	virtual void source(int, float, float) override;
	virtual void listener(float, float) override;

private:
	static void process_stream(void*);
	static void stream_drained(void*);
	void cleanup();

	pw_thread_loop *loop;
	/*
	pw_context *context;
	pw_core *core;
	pw_registry *registry;
	*/

	int next_id;
	float listener_x;
	float listener_y;
	SoundConfigFn config_fn;

	SoundBank soundbank;
	ActiveSoundStore<PipeWireActiveSound, 32> sounds;
};

}

#endif
