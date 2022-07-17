#include <string.h>

#include <win/pcmstream.hpp>
#include <win/ogg.hpp>

namespace win
{

PCMStream::PCMStream(win::PCMStreamCacheMode mode, std::int16_t *cache_buf, int cache_buf_len, Stream *oggstream)
	: cache_mode(mode)
	, cache_buf(cache_buf)
	, cache_buf_len(cache_buf_len)
	, cache_buf_filled(0)
	, channel_count(0)
	, writing_completed(false)
{
	if (mode == PCMStreamCacheMode::not_cached)
		decoder.start(std::move(*oggstream), *this, 0);
	else if (mode == PCMStreamCacheMode::partially_cached)
		decoder.start(std::move(*oggstream), *this, cache_buf_len);

	// the cache_buf contains data for us. otherwise we will fill up the cache buffer ourselves
	if (mode == PCMStreamCacheMode::partially_cached || mode == PCMStreamCacheMode::fully_cached)
	{
		if (ringbuffer.write(cache_buf, cache_buf_len) != cache_buf_len)
			win::bug("PCMStream: Couldn't populate cache");
	}
}

int PCMStream::read_samples(std::int16_t *dest, int len)
{
	return ringbuffer.read(dest, len);
}

int PCMStream::write_samples(const std::int16_t *source, int len)
{
	const int put = ringbuffer.write(source, len);

	if (cache_mode == PCMStreamCacheMode::not_cached )
	{
		const int filled = cache_buf_filled.load();
		const int put_cache = std::min(put, cache_buf_len - filled);
		memcpy(cache_buf + filled, source, put_cache * sizeof(std::int16_t));
		cache_buf_filled += put_cache;
	}

	return put;
}

int PCMStream::size() const
{
	return ringbuffer.size();
}

void PCMStream::complete_writing()
{
	writing_completed.store(true);
}

bool PCMStream::is_writing_completed() const
{
	return writing_completed.load();
}

void PCMStream::reset()
{
#ifndef NDEBUG
	if (!writing_completed.load())
		win::bug("Stream reset while streaming!");
#endif

	// rehydrate the cache
	if (ringbuffer.write(cache_buf, cache_buf_filled) != cache_buf_filled)
		win::bug("PCMStream reset:  Couldn't populate cache");

	writing_completed.store(false);
    decoder.reset(cache_buf_filled); // if the decoder isn't actually running, this does nothing. oh well
}

}
