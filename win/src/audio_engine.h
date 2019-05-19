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

struct audio_engine_remote;
class audio_engine
{
	friend display;

public:
	static constexpr int MAX_SOUNDS = 32;
	typedef void (*sound_config_fn)(float, float, float, float, float*, float*);

	audio_engine() = default;
	audio_engine(const audio_engine&) = delete;
	audio_engine(audio_engine&&);
	~audio_engine();

	void operator=(const audio_engine&) = delete;
	audio_engine &operator=(audio_engine&&);

	int play(sound&, bool = false); // ambient
	int play(sound&, float, float, bool = false); // stereo
	int play(sound&, bool, bool, float, float); // fully dressed function
	void pause();
	void resume();
	void pause(int);
	void resume(int);
	void source(int, float, float);
	void listener(float, float);

private:
	std::unique_ptr<audio_engine_remote> remote;

#if defined WINPLAT_WINDOWS
	audio_engine(sound_config_fn, display*);
	static void poke(audio_engine_remote*);
	void cleanup();
#elif defined WINPLAT_LINUX
	audio_engine(sound_config_fn);
	void cleanup(bool);
#endif
	void get_config(float, float, float, float, float*, float*);
	void finalize();
};

struct audio_engine_remote
{
	audio_engine_remote() = default;
	audio_engine_remote(const audio_engine_remote&) = delete;
	audio_engine_remote(audio_engine_remote&&) = delete;
	void operator=(audio_engine_remote&) = delete;
	void operator=(audio_engine_remote&&) = delete;

	int next_id_;
	float listener_x_;
	float listener_y_;
	win::audio_engine::sound_config_fn config_fn_;

#if defined WINPLAT_LINUX
	pa_context *context_;
	pa_threaded_mainloop *loop_;
	std::list<clip> clips_;
#elif defined WINPLAT_WINDOWS
	display *parent_;
	IDirectSound8 *context_;
	IDirectSoundBuffer *primary_;
	std::list<clip> clips_;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_poke_;
#endif
};

}

#endif
