#include <string.h>

#include <win/pcmresource.hpp>

namespace win
{

PCMResource::PCMResource(const char *name, int seek_start)
	: stream_name(name)
	, stream_seek_start(seek_start)
	, cache_fill(0)
	, completed(false)
	, channel_count(-1)
{
	cache_fill.store(0);
	completed.store(false);
}

void PCMResource::write_samples(const std::int16_t *samples, int len)
{
	if (is_completed())
		return;

	const int filled = cache_fill.load();
	const int avail = cache_len - filled;

	if (avail == 0)
	{
		complete();
		return;
	}

	if (avail < 0)
		win::bug("PCMResource: cache overwrite");

	const int put = std::min(len, avail);
	memcpy(pcm + filled, samples, put * sizeof(std::int16_t));

	cache_fill += put;
}

void PCMResource::set_channels(int c)
{
	channel_count.store(c);
}

const std::int16_t *PCMResource::data() const
{
	return pcm;
}

int PCMResource::fill() const
{
	return cache_fill.load();
}

const char *PCMResource::name() const
{
	return stream_name.c_str();
}

int PCMResource::seek_start() const
{
	return stream_seek_start;
}

int PCMResource::channels() const
{
	return channel_count.load();
}

bool PCMResource::is_completed() const
{
	return completed.load();
}

bool PCMResource::is_partial() const
{
	if (!completed.load())
		win::bug("PCMResource: not completed");

	return cache_fill.load() == cache_len;
}

void PCMResource::complete()
{
	completed.store(true);
	fprintf(stderr, "PCMResource: cache completed: fill %d\n", cache_fill.load());
}

}
