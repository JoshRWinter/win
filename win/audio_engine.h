#ifndef WIN_AUDIO_ENGINE_H
#define WIN_AUDIO_ENGINE_H

#include <list>

#include <pulse/pulseaudio.h>

namespace win
{

struct sound
{
#if defined WINPLAT_LINUX
	sound(audio_engine *parent_, int id_, bool looping_, unsigned long long start_, short *pcm_, std::atomic<unsigned long long> *size_, unsigned long long target_size_, pa_stream *stream_)
		: parent(parent_), id(id_), looping(looping_), start(start_), pcm(pcm_), size(size_), target_size(target_size_), drained(false), stream(stream_) {}
#endif
	audio_engine *parent;
	int id; // unique sound instance id
	bool looping;
	unsigned long long start; // start here next write
	short *pcm; // audio data
	std::atomic<unsigned long long> *size; // how much has been decoded
	unsigned long long target_size; // how big entire pcm buffer is

#if defined WINPLAT_LINUX
	std::atomic<bool> drained;
	pa_stream *stream;
#endif
};

class audio_engine
{
	friend display;

public:
	static constexpr int MAX_SOUNDS = 32;

	audio_engine(const audio_engine&) = delete;
	audio_engine(audio_engine&&);
	~audio_engine();

	void operator=(const audio_engine&) = delete;
	audio_engine &operator=(audio_engine&&);

	int play(const apack&, int, bool = false);
	void pause();
	void resume();
	void pause(int);
	void resume(int);
	void listener(float, float);

private:
	audio_engine();
	void move_platform(audio_engine&);
	void move_common(audio_engine&);
	void finalize();

	int next_id_;
	float listener_x_;
	float listener_y_;

#if defined WINPLAT_LINUX
	void cleanup(bool);

	pa_context *context_;
	pa_threaded_mainloop *loop_;
	std::list<sound> sounds_;
#endif
};

}

#endif