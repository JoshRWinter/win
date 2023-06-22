#pragma once

#include <thread>
#include <atomic>
#include <latch>

#include <win/Win.hpp>
#include <win/Stream.hpp>
#include <win/ConcurrentRingBuffer.hpp>
#include <win/sound/PcmSource.hpp>

namespace win
{

class DecodingPcmSource : public PcmSource
{
	WIN_NO_COPY_MOVE(DecodingPcmSource);
	constexpr static int buffersize = 44100 * 2 * 2; // 2 seconds of stereo, or 4 of mono

public:
	DecodingPcmSource(Stream data, int seek_start, int cached_channels, bool compute_total_size);
	~DecodingPcmSource();

	long pcm_size() const;
	void stop();

	int channels() override;
	void restart() override;
	bool empty() override;
	int read_samples(float *buf, int samples) override;

private:
	static long compute_size(Stream &stream);
	void set_channels(int c);
	static void decodeogg_loop(DecodingPcmSource &parent, win::Stream datafile, int seek_to);
	static void decodeogg(DecodingPcmSource &parent, win::Stream &datafile, int seek_to);

	win::ConcurrentRingBuffer<float, buffersize + 1> buffer;
	std::atomic<bool> cancel;
	std::atomic<bool> reset;
	std::atomic<bool> finished;
	std::atomic<int> channel_count;
	std::latch channels_initialized_signal;
	long total_size;
	std::thread worker;
};

}
