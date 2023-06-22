#pragma once

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/Pool.hpp>
#include <win/sound/Sound.hpp>
#include <win/sound/PcmSource.hpp>
#include <win/sound/DecodingPcmSource.hpp>
#include <win/sound/CachingPcmSource.hpp>
#include <win/sound/CachedPcmSource.hpp>

namespace win
{

struct SoundRepoCacheEntry
{
	WIN_NO_COPY_MOVE(SoundRepoCacheEntry);

	SoundRepoCacheEntry(const char *name, int seek)
		: key_name(name)
		, key_seek(seek)
		, channels(-1)
		, length(-1)
	{}

	std::string key_name;
	int key_seek;

	int channels;
	long length;
	std::unique_ptr<const float[]> pcm;
};

struct SoundRepoEntry : Sound
{
	WIN_NO_COPY_MOVE(SoundRepoEntry);

	SoundRepoEntry(PcmSource &source, SoundRepoCacheEntry &cache_entry, DecodingPcmSource *decoder, CachingPcmSource *cacher, CachedPcmSource *cached)
		: Sound(source)
		, cache_entry(cache_entry)
		, decoder(decoder)
		, cacher(cacher)
		, cached(cached)
	{}

	SoundRepoCacheEntry &cache_entry;

	DecodingPcmSource *decoder;
	CachingPcmSource *cacher;
	CachedPcmSource *cached;
};

class SoundRepo
{
	WIN_NO_COPY_MOVE(SoundRepo);

public:
	explicit SoundRepo(win::AssetRoll&);
	~SoundRepo();

	Sound &load(const char *name, bool cache, int seek);
	void unload(Sound &sound);

private:
	AssetRoll &roll;
	Pool<SoundRepoEntry, 25> entries;
	Pool<SoundRepoCacheEntry, 25> cache_entries;
	Pool<DecodingPcmSource, 25> decoders;
	Pool<CachingPcmSource, 25> caching_sources;
	Pool<CachedPcmSource, 25> cached_sources;
};

}
