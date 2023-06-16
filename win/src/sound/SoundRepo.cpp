#include <win/sound/SoundRepo.hpp>

namespace win
{

SoundRepo::SoundRepo(win::AssetRoll &roll)
	: roll(roll)
{}

SoundRepo::~SoundRepo()
{
	if (entries.size() != 0)
		win::bug("SoundRepo: Leftover sound entries");
	if (decoders.size() != 0)
		win::bug("SoundRepo: Leftover decoders");
	if (caching_sources.size() != 0)
		win::bug("SoundRepo: Leftover caching sources");
	if (cached_sources.size() != 0)
		win::bug("SoundRepo: Leftover cached sources");
}

Sound &SoundRepo::load(const char *name, bool cache, int seek)
{
	SoundRepoCacheEntry *cached = NULL;
	// create a cache entry if one doesn't exist yet
	for (auto &entry: cache_entries)
	{
		if (entry.key_name == name && entry.key_seek == seek)
		{
			cached = &entry;
			break;
		}
	}

	if (cached == NULL)
		cached = &cache_entries.add(name, seek);

	if (cached->pcm) // if there is cached sound data
	{
		auto &cached_source = cached_sources.add(cached->channels, cached->pcm.get(), cached->length);

		return entries.add((PcmSource&) cached_source, *cached, (DecodingPcmSource *)NULL, (CachingPcmSource *)NULL, &cached_source);
	}
	else if (cache) // no cached data exists, but we want to cache
	{
		auto &decoder = decoders.add(roll[name], seek, cached->channels, true);

		const long size = std::max(0L, decoder.pcm_size() - seek);
		float *destination = new float[size];

		auto &cached_source = cached_sources.add(decoder.channels(), destination, size);
		auto &caching_source = caching_sources.add(decoder, cached_source, destination, size);

		return entries.add(caching_source, *cached, &decoder, &caching_source, &cached_source);
	}
	else // no cache pls
	{
		auto &decoder = decoders.add(roll[name], seek, cached->channels, true);

		return entries.add(decoder, *cached, &decoder, (CachingPcmSource *)NULL, (CachedPcmSource *)NULL);
	}
}

void SoundRepo::unload(Sound &sound)
{
	auto &entry = (SoundRepoEntry &)sound;

	// fill in the cache information, if appropriate

	if (entry.cache_entry.channels == -1)
	{
		entry.cache_entry.channels = sound.source.channels();
	}

	if (!entry.cache_entry.pcm && entry.cacher != NULL && entry.cacher->is_complete())
	{
		const float *const pcm = entry.cacher->release_pcm_data();
		const long length = entry.cacher->pcm_length();

		entry.cache_entry.pcm.reset(pcm);
		entry.cache_entry.length = length;
	}

	if (entry.cacher != NULL)
		caching_sources.remove(*entry.cacher);

	if (entry.decoder != NULL)
		decoders.remove(*entry.decoder);

	if (entry.cached != NULL)
		cached_sources.remove(*entry.cached);

	entries.remove(entry);
}

}
