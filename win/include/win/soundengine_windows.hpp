#ifndef WIN_SOUNDENGINE_WINDOWS_HPP
#define WIN_SOUNDENGINE_WINDOWS_HPP

#include <win/win.hpp>

#ifdef WINPLAT_WINDOWS

#include <mmsystem.h>
#include <dsound.h>

#include <win/soundenginecommon.hpp>
#include <win/soundbank.hpp>
#include <win/sound.hpp>

namespace win
{

struct DirectSoundActiveSound
{
	WIN_NO_COPY_MOVE(DirectSoundActiveSound);

	DirectSoundActiveSound(Sound &sound, IDirectSoundBuffer8 *stream, int sid, bool ambient, bool looping, float x, float y)
		: sound(&sound), id(sid), ambient(ambient), looping(looping), x(x), y(y), stream(stream), stop(false), write_cursor(0), silent_samples_written(0), firstwrite(true) {}

	Sound *sound;
	int id;
	bool ambient;
	bool looping;
	float x,y;
	bool stop;

	IDirectSoundBuffer8 *stream;
	unsigned long long write_cursor;
	unsigned long long silent_samples_written;
	bool firstwrite;
};

class Display;
class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

	static constexpr unsigned long long SOUND_BUFFER_BYTES = 1 * 44100 * 2 * sizeof(std::int16_t); // seconds * sample rate * channels * sample size
	static constexpr unsigned long long SOUND_BUFFER_SAMPLES = SOUND_BUFFER_BYTES / sizeof(std::int16_t); // seconds * sample rate * sample size
	static constexpr unsigned long long MAX_WRITE_BYTES = SOUND_BUFFER_BYTES; // half the sound buffer size
	static constexpr unsigned long long MAX_WRITE_SAMPLES = MAX_WRITE_BYTES / sizeof(std::int16_t); // half the sound buffer size

public:
	SoundEngine(Display&, AssetRoll&, SoundConfigFn);
	~SoundEngine();

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

	void write_to_stream(DirectSoundActiveSound&);
	unsigned long long write_empty(DirectSoundActiveSound&, unsigned long long);

	IDirectSound8 *context;
	IDirectSoundBuffer *primary;
	constexpr static unsigned long long CONVBUFFER_SAMPLES = (MAX_WRITE_SAMPLES / 2) * 3;
	std::unique_ptr<std::int16_t[]> convbuffer;

	int next_id;
	float listener_x;
	float listener_y;
	SoundConfigFn config_fn;
	//std::chrono::time_point<std::chrono::high_resolution_clock> last_process;

	SoundBank soundbank;
	std::list<DirectSoundActiveSound> sounds;
};

}

#endif

#endif