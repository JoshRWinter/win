#ifndef WIN_SOUND_ENGINE_HPP
#define WIN_SOUND_ENGINE_HPP

#include <list>
#include <memory>

#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <pulse/pulseaudio.h>
#elif defined WINPLAT_WINDOWS
#include <mmsystem.h>
#include <dsound.h>
#endif

#include <win/soundbank.hpp>


namespace win
{

struct ActiveSound
{
#if defined WINPLAT_LINUX
	ActiveSound(Sound &sound, pa_stream *stream, int sid, bool ambient, bool looping, float x, float y)
		: sound(&sound), id(sid), ambient(ambient), looping(looping), x(x), y(y), stream(stream), drain_op(NULL), drained(false), stop(false) {}
#elif defined WINPLAT_WINDOWS
	ActiveSound(Sound &sound, IDirectSoundBuffer8 *stream, int sid, bool ambient, bool looping, float x, float y)
		: sound(&sound), id(sid), ambient(ambient), looping(looping), x(x), y(y), stream(stream), stop(false), write_cursor(0), silent_samples_written(0), firstwrite(true) {}
#endif

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

#if defined WINPLAT_LINUX
	pa_stream *stream;
	pa_operation *drain_op;
	std::atomic<bool> drained;
#elif defined WINPLAT_WINDOWS
	IDirectSoundBuffer8 *stream;
	unsigned long long write_cursor;
	unsigned long long silent_samples_written;
	bool firstwrite;
#endif
};

class Display;
class SoundEngine
{
	typedef void (*SoundConfigFn)(float, float, float, float, float*, float*);
	static constexpr int MAX_SOUNDS = 32;

    static void channel_dupe(std::int16_t *const dest, const std::int16_t *const source, const size_t source_len)
    {
	    for(size_t i = 0; i < source_len * 2; i += 2)
	    {
		    dest[i] = source[i / 2];
		    dest[i + 1] = source[i / 2];
	    }
    }

    static void default_sound_config_fn(float, float, float, float, float *volume, float *balance)
    {
	    *volume = 1.0f;
	    *balance = 0.0f;
    }

    static float clamp_volume(float v)
    {
	    if(v > 1.0f)
		    return 1.0f;
	    else if(v < 0.0f)
		    return 0.0f;

	    return v;
    }

    static float clamp_balance(float bal)
    {
	    if(bal > 1.0f)
		    return 1.0f;
	    else if(bal < -1.0f)
		    return -1.0f;

	    return bal;
    }

    void get_config(float listenerx, float listenery, float sourcex, float sourcey, float *volume_l, float *volume_r)
    {
	    float volume = 0.0f, balance = 0.0f;

	    config_fn(listenerx, listenery, sourcex, sourcey, &volume, &balance);

	    // clamp [0.0, 1.0f]
	    volume = clamp_volume(volume);

	    // clamp [-1.0f, 1.0f]
	    balance = clamp_balance(balance);

	    // convert to volumes
	    *volume_l = volume;
	    *volume_r = volume;

	    if(balance > 0.0f)
		    *volume_l -= balance;
	    else if(balance < 0.0f)
		    *volume_r += balance;

	    // reclamp
	    *volume_l = clamp_volume(*volume_l);
	    *volume_r = clamp_volume(*volume_r);
    }

#ifdef WINPLAT_WINDOWS
	static constexpr unsigned long long SOUND_BUFFER_BYTES = 1 * 44100 * 2 * sizeof(std::int16_t); // seconds * sample rate * channels * sample size
	static constexpr unsigned long long SOUND_BUFFER_SAMPLES = SOUND_BUFFER_BYTES / sizeof(std::int16_t); // seconds * sample rate * sample size
	static constexpr unsigned long long MAX_WRITE_BYTES = SOUND_BUFFER_BYTES; // half the sound buffer size
	static constexpr unsigned long long MAX_WRITE_SAMPLES = MAX_WRITE_BYTES / sizeof(std::int16_t); // half the sound buffer size
#endif

public:
	SoundEngine(Display&, AssetRoll&, SoundConfigFn);
	~SoundEngine();

	WIN_NO_COPY_MOVE(SoundEngine);

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
	void cleanup(bool);

#if defined WINPLAT_WINDOWS
	void write_to_stream(ActiveSound&);
	unsigned long long write_empty(ActiveSound&, unsigned long long);

	IDirectSound8 *context;
	IDirectSoundBuffer *primary;
	constexpr static unsigned long long CONVBUFFER_SAMPLES = (MAX_WRITE_SAMPLES / 2) * 3;
	std::unique_ptr<std::int16_t[]> convbuffer;

#elif defined WINPLAT_LINUX
	void write_to_stream(ActiveSound&, size_t);

	unsigned long long conversion_buffer_sample_count;
	std::unique_ptr<std::int16_t[]> conversion_buffer;
	pa_context *context;
	pa_threaded_mainloop *loop;
#endif

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
