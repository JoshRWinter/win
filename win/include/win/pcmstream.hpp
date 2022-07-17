#ifndef WIN_SOUND_STREAM_HPP
#define WIN_SOUND_STREAM_HPP

#include <atomic>
#include <thread>

#include <win/win.hpp>
#include <win/stream.hpp>
#include <win/ringbuffer.hpp>
#include <win/pcmdecoder.hpp>

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
	friend class PCMStreamCache;
	static constexpr int ringbuf_size = (44100 * 5) + 1; // n - 1 needs to be divisible by both 1 and 2
	WIN_NO_COPY_MOVE(PCMStream);

public:
	PCMStream(PCMStreamCacheMode, std::int16_t*, int, Stream*);

	int read_samples(std::int16_t*, int);
	int write_samples(const std::int16_t*, int);
	int size() const;
	void complete_writing();
	bool is_writing_completed() const;
	void reset();
	int channels() const { return channel_count; }
	void set_channels(int channel_count) { this->channel_count = channel_count; }

private:
	int write_samples_impl(const std::int16_t*, int);

	const PCMStreamCacheMode cache_mode;
	std::int16_t *const cache_buf;
	const int cache_buf_len;
	std::atomic<int> cache_buf_filled;

	int channel_count;

	std::atomic<bool> writing_completed;
	win::ConcurrentRingBuffer<std::int16_t, ringbuf_size> ringbuffer;

	win::PCMDecoder decoder;
};

}

#endif
