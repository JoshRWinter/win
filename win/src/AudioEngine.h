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

class audio_engine;
struct clip
{
#if defined WINPLAT_LINUX
	clip(int id_, bool looping_, unsigned long long start_, short *pcm_, std::atomic<unsigned long long> *size_, unsigned long long target_size_, pa_stream *stream_, bool ambient_, float x_, float y_)
		: id(id_), looping(looping_), start(start_), pcm(pcm_), size(size_), target_size(target_size_), ambient(ambient_), x(x_), y(y_), drain(NULL), stream(stream_), finished(false) {}
#elif defined WINPLAT_WINDOWS
	clip(int id_, bool looping_, unsigned long long start_, short *pcm_, std::atomic<unsigned long long> *size_, unsigned long long target_size_, bool ambient_, float x_, float y_, IDirectSoundBuffer8 *stream_)
		:id(id_), looping(looping_), start(start_), pcm(pcm_), size(size_), target_size(target_size_), ambient(ambient_), x(x_), y(y_), stream(stream_), write_cursor(0) {}
	void finalize() { stream->Stop(); stream->Release(); }
#endif
	int id; // unique sound instance id
	bool looping;
	unsigned long long start; // start here next write
	const short *pcm; // audio data
	std::atomic<unsigned long long> *size; // how much has been decoded
	unsigned long long target_size; // how big entire pcm buffer is
	bool ambient; // is not affected by world position
	float x, y; // position in the world

#if defined WINPLAT_LINUX
	pa_stream *stream;
	pa_operation *drain;
	std::atomic<bool> finished;
#elif defined WINPLAT_WINDOWS
	IDirectSoundBuffer8 *stream;
	int write_cursor;
#endif
};

class Display;

class AudioEngine
{
	typedef void (*SoundConfigFn)(float, float, float, float, float*, float*);
	static constexpr int MAX_SOUNDS = 32;

public:
	AudioEngine();
	AudioEngine(const Display&, SoundConfigFn);
	AudioEngine(const AudioEngine&) = delete;
	AudioEngine(AudioEngine&&);
	~AudioEngine();

	void operator=(const AudioEngine&) = delete;
	AudioEngine &operator=(AudioEngine&&);

	int play(Sound&, bool = false); // ambient
	int play(Sound&, float, float, bool = false); // stereo
	int play(Sound&, bool, bool, float, float); // fully dressed function
	void pause();
	void resume();
	void pause(int);
	void resume(int);
	void source(int, float, float);
	void listener(float, float);

private:
	void finalize();
	void move(AudioEngine&);

#if defined WINPLAT_WINDOWS
	static void poke(audio_engine_remote*);
	void cleanup();

	IDirectSound8 *context;
	IDirectSoundBuffer *primary;
	std::list<clip> clips;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_poke;
#elif defined WINPLAT_LINUX
	void cleanup(bool);

	pa_context *context;
	pa_threaded_mainloop *loop;
	std::list<clip> clips;
#endif
	void get_config(float, float, float, float, float*, float*);

	const Display *parent;
	int next_id;
	float listener_x;
	float listener_y;
	SoundConfigFn config_fn;

};

}

#endif
