#include <win/pcmstreamcache.hpp>

namespace win
{

PCMStreamCache::PCMStreamCache(win::AssetRoll &roll)
	: roll(roll)
{}

PCMStreamCache::~PCMStreamCache()
{
	if (loaded_streams.size() != 0)
		win::bug("Leftover sound streams");
}

PCMStream &PCMStreamCache::load_stream(const char *name)
{
	// see if we've got this one
    CachedPCMStream *found = NULL;
    for (CachedPCMStream &c : cached_streams)
	{
		if (c.name == name)
		{
			found = &c;
			break;
		}
	}

	if (found == NULL)
	{
		fprintf(stderr, "NO CACHE\n");
		// no cache entry yet

		Stream s = roll[name];
		// the win::PCMStream will std::move the "s" into itself
		return loaded_streams.add(PCMStreamCacheMode::not_cached, cached_streams.add(name).pcm.data(), CachedPCMStream::pcm_cache_len, &s);
	}
	else if (!found->completed)
	{
		fprintf(stderr, "CACHE PRESENT, BUT INCOMPLETE\n");
		// there is a cache entry, but it isn't done yet. don't use it, and don't make a new one

		Stream s = roll[name];
		// the win::PCMStream will std::move the "s" into itself
		return loaded_streams.add(PCMStreamCacheMode::not_cached, (std::int16_t*)NULL, 0, &s);
	}
	else if (found->fully_cached)
	{
		fprintf(stderr, "FULL CACHE PRESENT\n");
    	return loaded_streams.add(PCMStreamCacheMode::fully_cached, found->pcm.data(), found->cache_fill, (Stream*)NULL);
	}
	else
	{
		fprintf(stderr, "PARTIALLY CACHED\n");
		// partially cached

		Stream s = roll[name];
		// the win::PCMStream will std::move the "s" into itself
		return loaded_streams.add(PCMStreamCacheMode::partially_cached, found->pcm.data(), CachedPCMStream::pcm_cache_len, &s);
	}
}

void PCMStreamCache::unload_stream(PCMStream &stream)
{
	// see about saving some of this stream's data to the cache

	if (stream.cache_mode != PCMStreamCacheMode::not_cached)
	{
		fprintf(stderr, "NOT SAVING CACHE\n");
		// early out
		loaded_streams.remove(stream);
		return;
	}

	// find the cache buf associated with this stream
	CachedPCMStream *found = NULL;
	for (CachedPCMStream &c : cached_streams)
	{
		if (stream.cache_buf == c.pcm.data())
		{
			found = &c;
			break;
		}
	}

	if (found != NULL)
	{
		if (stream.is_writing_completed())
		{
			fprintf(stderr, "FOUND EXISTING CACHE, COMPLETING\n");
			const bool fully_cached = found->cache_fill < CachedPCMStream::pcm_cache_len;
			found->complete(fully_cached, stream.cache_buf_filled.load());
		}
		else
		{
			fprintf(stderr, "FOUND EXISTING CACHE, DISCARDING\n");
			// delete the cache entry, it turned out to be a waste of time
			cached_streams.remove(*found);
		}
	}
	else fprintf(stderr, "FOUND EXISTING CACHE, DISCARDING\n");

	loaded_streams.remove(stream);
}

}
