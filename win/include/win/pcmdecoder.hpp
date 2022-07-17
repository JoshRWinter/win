#ifndef WIN_PCM_DECODER_HPP
#define WIN_PCM_DECODER_HPP

#include <condition_variable>
#include <thread>
#include <atomic>

#include <win/win.hpp>
#include <win/stream.hpp>

namespace win
{

class PCMStream;
class PCMDecoder
{
	WIN_NO_COPY_MOVE(PCMDecoder);
public:
	PCMDecoder();
	~PCMDecoder();

    void start(win::Stream, win::PCMStream&, int);
	void reset(int);

private:
	std::atomic<bool> cancel;
	std::atomic<bool> restart;
	std::atomic<int> seek_to;
	std::thread worker;
};

}

#endif
