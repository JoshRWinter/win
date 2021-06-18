#ifndef WIN_AUDIO_ENGINE_H
#define WIN_AUDIO_ENGINE_H

#include <list>
#include <chrono>
#include <memory>

#if defined WINPLAT_LINUX
#include <pulse/pulseaudio.h>
#elif defined WINPLAT_WINDOWS
#include <dsound.h>
#endif

namespace win
{

struct ActiveSound
{
	ActiveSound(Sound &sound, pa_stream *stream, int sid, bool ambient, bool looping, float x, float y)
		: sound(&sound), id(sid), ambient(ambient), looping(looping), x(x), y(y), stream(stream), drain_op(NULL), drained(false), stop(false) {}

	ActiveSound(const ActiveSound&) = delete;
	ActiveSound(ActiveSound&&) = delete;

	void operator=(const ActiveSound&) = delete;
	void operator=(ActiveSound&&) = delete;

	Sound *sound;
	int id;
	bool ambient;
	bool looping;
	float x,y;
	bool stop;

#ifdef WINPLAT_LINUX
	pa_stream *stream;
	pa_operation *drain_op;
	std::atomic<bool> drained;
#endif
};

class Display;
class SoundEngine
{
	typedef void (*SoundConfigFn)(float, float, float, float, float*, float*);
	static constexpr int MAX_SOUNDS = 32;

public:
	SoundEngine(Display&, AssetRoll&, SoundConfigFn);
	SoundEngine(const SoundEngine&) = delete;
	SoundEngine(SoundEngine&&) = delete;
	~SoundEngine();

	void operator=(const SoundEngine&) = delete;
	void operator=(SoundEngine&&) = delete;

	void process();

	int play(const char*, bool = false); // ambient
	int play(const char*, float, float, bool = false); // stereo
	int play(const char*, bool, bool, float, float); // fully dressed function
	void pause(int);
	void resume(int);
	void stop(int);
	void source(int, float, float);
	void listener(float, float);


private:
#if defined WINPLAT_WINDOWS
	void poke();
	void cleanup();

	IDirectSound8 *context;
	IDirectSoundBuffer *primary;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_poke;
#elif defined WINPLAT_LINUX
	void write_to_stream(ActiveSound&, size_t);
	void cleanup(bool);

	unsigned long long sample_buffer_samples_count;
	std::unique_ptr<std::int16_t[]> sample_buffer;
	pa_context *context;
	pa_threaded_mainloop *loop;
#endif
	void get_config(float, float, float, float, float*, float*);

	int next_id;
	float listener_x;
	float listener_y;
	SoundConfigFn config_fn;
	//std::chrono::time_point<std::chrono::high_resolution_clock> last_process;

	SoundBank soundbank;
	std::list<ActiveSound> sounds;
};

}

#endif
