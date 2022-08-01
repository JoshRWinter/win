#ifndef WIN_PCM_RESOURCE_HPP
#define WIN_PCM_RESOURCE_HPP

#include <atomic>
#include <string>

#include <win/win.hpp>

namespace win
{

class PCMResource
{
	WIN_NO_COPY_MOVE(PCMResource);

    static constexpr int cache_len = 44100 * 5; // 5 seconds of mono, 2.5 of stereo

public:
    PCMResource(const char*, int);

    void write_samples(const std::int16_t*, int len);
	void set_channels(int);
	const std::int16_t *data() const;
    int fill() const;
	const char *name() const;
	int seek_start() const;
	int channels() const;
    bool is_completed() const;
	bool is_partial() const;
	void complete();

private:
	std::string stream_name;
	int stream_seek_start;
   	std::int16_t pcm[cache_len];
	std::atomic<int> cache_fill;
    std::atomic<bool> completed;
	std::atomic<int> channel_count;
};

}

#endif
