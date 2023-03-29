#pragma once

#include <atomic>
#include <thread>

#include <win/Win.hpp>
#include <win/Stream.hpp>
#include <win/ConcurrentRingBuffer.hpp>
#include <win/sound/FloatPcmProvider.hpp>
#include <win/sound/PcmDecoder.hpp>

namespace win
{

class PcmStream : public FloatPcmProvider
{
	static constexpr int ringbuf_size = (44100 * 5) + 1; // n - 1 needs to be divisible by both 1 and 2
	WIN_NO_COPY_MOVE(PcmStream);

public:
	PcmStream();

	int read_samples(float*, int) override;
	int read_samples(std::int16_t*, int);
	int write_samples(const std::int16_t*, int);
	int size() const;
	void complete_writing();
	bool is_writing_completed() const;
	void reset();
	int channels() const { return channel_count.load(); }
	void set_channels(int c) { this->channel_count.store(c); }

private:
	std::atomic<int> channel_count;
	std::atomic<bool> writing_completed;
	win::ConcurrentRingBuffer<std::int16_t, ringbuf_size> ringbuffer;
};

}
