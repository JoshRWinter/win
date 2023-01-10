#pragma once

#include <condition_variable>
#include <thread>
#include <atomic>

#include <win/win.hpp>
#include <win/stream.hpp>
#include <win/sound/pcmstream.hpp>
#include <win/sound/pcmresource.hpp>

namespace win
{

namespace impl
{

// the ergonomics of std::condition_variable are feckin terrible
class OneshotSignal
{
public:
	OneshotSignal()
		: signaled(false)
		, dirty(false)
    {}

	void wait()
	{
		if (dirty)
			win::bug("Dirty oneshot");

		std::unique_lock<std::mutex> lock(mutex);
		cvar.wait(lock, [this]() { return signaled == true; });
		dirty = true;
	}

	void notify()
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			signaled = true;
		}

		cvar.notify_one();
	}

private:
	bool signaled;
	bool dirty;
	std::mutex mutex;
	std::condition_variable cvar;
};

}

class PCMStream;
class PCMDecoder
{
	WIN_NO_COPY_MOVE(PCMDecoder);
public:
	PCMDecoder(PCMStream&, PCMResource&, Stream*, int);
	~PCMDecoder();

	void reset();
	void stop();
	PCMResource &resource();

private:
	int write_samples(const std::int16_t*, int);
	void set_channels(int);
	void complete_writing();

	static void decodeogg_loop(PCMDecoder&, win::Stream, impl::OneshotSignal*);
	static void decodeogg(PCMDecoder&, win::Stream&, int, impl::OneshotSignal*);

	PCMStream &target;
	PCMResource &pcmresource;
	int seek_start;
	std::atomic<bool> cancel;
	std::atomic<bool> restart;
	std::atomic<int> seek_to;
	std::thread worker;
};

}
