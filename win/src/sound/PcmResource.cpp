#include <string.h>
#include <algorithm>

#include <win/sound/PcmResource.hpp>

namespace win
{

PcmResource::PcmResource(const char *name, int seek_start)
	: stream_name(name)
	, stream_seek_start(seek_start)
	, cache_fill(0)
	, completed(false)
	, channel_count(-1)
	, finalized(false)
{
	cache_fill.store(0);
	completed.store(false);
}

PcmResource::PcmResource(PcmResource &&rhs) noexcept
	: stream_name(std::move(rhs.stream_name))
	, stream_seek_start(rhs.stream_seek_start)
	, cache_fill(rhs.cache_fill.load())
	, completed(rhs.completed.load())
	, channel_count(rhs.channel_count.load())
	, finalized(rhs.finalized.load())
{
	memcpy(pcm, rhs.pcm, sizeof(pcm));
}

void PcmResource::write_samples(const std::int16_t *samples, int len)
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

void PcmResource::set_channels(int c)
{
	channel_count.store(c);
}

const std::int16_t *PcmResource::data() const
{
	return pcm;
}

int PcmResource::fill() const
{
	return cache_fill.load();
}

const char *PcmResource::name() const
{
	return stream_name.c_str();
}

int PcmResource::seek_start() const
{
	return stream_seek_start;
}

int PcmResource::channels() const
{
	return channel_count.load();
}

bool PcmResource::is_completed() const
{
	return completed.load();
}

bool PcmResource::is_finalized() const
{
	return finalized.load();
}

bool PcmResource::is_partial() const
{
	if (!completed.load())
		win::bug("PCMResource: not completed");

	return cache_fill.load() == cache_len;
}

void PcmResource::complete()
{
	completed.store(true);
}

void PcmResource::finalize()
{
	finalized.store(true);
}

}
