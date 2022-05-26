#ifndef WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_HPP
#define WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pulse/pulseaudio.h>

#include <win/soundengine_linux_pulseaudio_functions.hpp>
#include <win/soundengine_linux.hpp>
#include <win/sound.hpp>
#include <win/soundbank.hpp>

namespace win
{

/*
struct PulseAudioActiveSound
{
	WIN_NO_COPY_MOVE(PulseAudioActiveSound);

	PulseAudioActiveSound(Sound &sound, pa_stream *stream, int sid, bool ambient, bool looping, float x, float y)
		: sound(&sound), id(sid), ambient(ambient), looping(looping), x(x), y(y), stream(stream), drain_op(NULL), drained(false), stop(false) {}

	Sound *sound;
	int id;
	bool ambient;
	bool looping;
	float x,y;
	bool stop;

	pa_stream *stream;
	pa_operation *drain_op;
	std::atomic<bool> drained;
};

class SoundEngineLinuxPulseAudio : public SoundEngineLinuxProxy
{
public:
	SoundEngineLinuxPulseAudio(Display&, AssetRoll&, SoundConfigFn);
	virtual ~SoundEngineLinuxPulseAudio() override;

	WIN_NO_COPY_MOVE(SoundEngineLinuxPulseAudio);

	virtual void process() override;

	virtual int play(const char*, bool = false) override;
	virtual int play(const char*, float, float, bool = false) override;
	virtual int play(const char*, float, float, bool, bool) override;
	virtual void pause(int) override;
	virtual void resume(int) override;
	virtual void stop(int) override;
	virtual void source(int, float, float) override;
	virtual void listener(float, float) override;


private:
	void cleanup(bool);
	void write_to_stream(PulseAudioActiveSound&, size_t);

	unsigned long long conversion_buffer_sample_count;
	std::unique_ptr<std::int16_t[]> conversion_buffer;
	pa_context *context;
	pa_threaded_mainloop *loop;

	int next_id;
	float listener_x;
	float listener_y;
	SoundConfigFn config_fn;

	SoundBank soundbank;
	std::list<PulseAudioActiveSound> sounds;
};
*/

}

#endif

#endif
