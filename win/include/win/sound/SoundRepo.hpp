#pragma once

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/Pool.hpp>
#include <win/sound/Sound.hpp>
#include <win/sound/PcmSource.hpp>
#include <win/sound/DecodingPcmSource.hpp>

namespace win
{

struct SoundRepoCacheEntry
{
	SoundRepoCacheEntry(const char *name, int seek)
		: key_name(name)
		, key_seek(seek)
		, channels(-1)
		, length_samples(-1)
	{}

	std::string key_name;
	int key_seek;

	int channels;
	long length_samples;
	std::unique_ptr<float[]> pcm;
};

struct SoundRepoEntry : Sound
{
	SoundRepoEntry(PcmSource &source, SoundRepoCacheEntry *cache_entry)
		: Sound(source)
		, cache_entry(cache_entry)
	{}

	SoundRepoCacheEntry *cache_entry;
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
};

}
