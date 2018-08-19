#ifndef WIN_AUDIO_ENGINE_H
#define WIN_AUDIO_ENGINE_H

#include <list>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>

#include <pulse/pulseaudio.h>

namespace win
{

class audio_engine;
struct sound
{
#if defined WINPLAT_LINUX
	sound(audio_engine *parent_, int id_, bool looping_, unsigned long long start_, short *pcm_, std::atomic<unsigned long long> *size_, unsigned long long target_size_, pa_stream *stream_, bool ambient_, float x_, float y_)
		: parent(parent_), id(id_), looping(looping_), start(start_), pcm(pcm_), size(size_), target_size(target_size_), ambient(ambient_), x(x_), y(y_), drained(false), stream(stream_) {}
#endif
	audio_engine *parent;
	int id; // unique sound instance id
	bool looping;
	unsigned long long start; // start here next write
	short *pcm; // audio data
	std::atomic<unsigned long long> *size; // how much has been decoded
	unsigned long long target_size; // how big entire pcm buffer is
	bool ambient; // is not affected by world position
	float x, y; // position in the world

#if defined WINPLAT_LINUX
	std::atomic<bool> drained;
	pa_stream *stream;
#endif
};

struct stored_sound
{
	std::atomic<unsigned long long> size;
	std::unique_ptr<short[]> buffer;
	std::unique_ptr<unsigned char[]> encoded;
	unsigned long long target_size;
	std::thread thread;
};

struct apack
{
	std::unique_ptr<stored_sound[]> stored;
	int count;
};

class audio_engine
{
	friend display;

public:
	static constexpr int MAX_SOUNDS = 32;
	typedef void (*sound_config_fn)(float, float, float, float, float*, float*);

	audio_engine();
	audio_engine(const audio_engine&) = delete;
	audio_engine(audio_engine&&);
	~audio_engine();

	void operator=(const audio_engine&) = delete;
	audio_engine &operator=(audio_engine&&);

	void import(const data_list&);
	int play(int, bool = false, int = 0); // ambient
	int play(int, float, float, bool = false, int = 0); // stereo
	int play(int, int, bool, bool, float, float); // fully dressed function
	void pause();
	void resume();
	void pause(int);
	void resume(int);
	void source(int, float, float);
	void listener(float, float);

private:
	audio_engine(sound_config_fn);
	void get_config(float, float, float, float, float*, float*);
	void move_platform(audio_engine&);
	void move_common(audio_engine&);
	void finalize();

	int next_id_;
	float listener_x_;
	float listener_y_;
	sound_config_fn config_fn_;
	std::vector<apack> imported_;

#if defined WINPLAT_LINUX
	void cleanup(bool);

	pa_context *context_;
	pa_threaded_mainloop *loop_;
	std::list<sound> sounds_;
#endif
};

}

#endif
