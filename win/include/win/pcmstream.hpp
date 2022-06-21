#ifndef WIN_SOUND_STREAM_HPP
#define WIN_SOUND_STREAM_HPP

#include <atomic>
#include <thread>

#include <win/win.hpp>
#include <win/stream.hpp>
#include <win/ringbuffer.hpp>

namespace win
{

enum class PCMStreamCacheMode
{
	not_cached,
	partially_cached,
	fully_cached
};

class PCMStreamCache;
class PCMStream
{
	static constexpr int ringbuf_size = 4096;
	WIN_NO_COPY_MOVE(PCMStream);

public:
	PCMStream(PCMStreamCache*, Stream*, PCMStreamCacheMode);
	~PCMStream();

	int read_samples(std::int16_t*, int);
	int write_samples(const std::int16_t*, int);
	int size() const;
	void complete_writing();
	bool is_writing_completed() const;
	int channels() const { return channel_count; }

	std::atomic<bool> decoder_cancel;

private:
	int write_samples_impl(const std::int16_t*, int);

	PCMStreamCache *parent;
	PCMStreamCacheMode kind;

public:
	int channel_count;
private:
	std::atomic<bool> writing_completed;
	std::thread decoder_thread;
	win::ConcurrentRingBuffer<std::int16_t, ringbuf_size> ringbuffer;
};

}

#endif
