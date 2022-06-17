#ifndef WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP
#define WIN_SOUND_ENGINE_LINUX_PIPEWIRE_HPP

#include <pipewire/pipewire.h>

#include <win/activesoundstore.hpp>
#include <win/soundengine_linux.hpp>

namespace win
{

/*
struct PipeWireActiveSound
{
	WIN_NO_COPY_MOVE(PipeWireActiveSound);

	PipeWireActiveSound(Sound &sound)
		: sound(&sound)
		, stream(NULL)
		, done(false)
		, vol_left(1.0f)
		, vol_right(1.0f)
	{
		memset(&events, 0, sizeof(events));
	}

	Sound *sound;
	pw_stream_events events;
	pw_stream *stream;
	bool done;
	float vol_left;
	float vol_right;
};

class SoundEngineLinuxPipeWire : public SoundEngineLinuxProxy
{
public:
	SoundEngineLinuxPipeWire(/*Display&, AssetRoll&);
	virtual ~SoundEngineLinuxPipeWire() override;

	WIN_NO_COPY_MOVE(SoundEngineLinuxPipeWire);

	virtual std::uint32_t play(const char*, bool = false) override;
	virtual std::uint32_t play(const char*, float, float, bool = false) override;
	virtual std::uint32_t play(const char*, float, float, bool, bool) override;
	virtual void pause(std::uint32_t) override;
	virtual void resume(std::uint32_t) override;
	virtual void stop(std::uint32_t) override;
	virtual void config(std::uint32_t, float, float) override;

private:
	static void stream_process(void*);
	static void stream_drained(void*);
	static void stream_state_changed(void*);
	void cleanup(bool);

	pw_thread_loop *loop;
	/*
	pw_context *context;
	pw_core *core;
	pw_registry *registry;


	int next_id;
	float listener_x;
	float listener_y;

	SoundBank soundbank;
	ActiveSoundStore<PipeWireActiveSound, 32> sounds;
};
*/

}

#endif
