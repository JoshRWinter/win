#include <win/sound/SoundRepo.hpp>

namespace win
{

SoundRepo::SoundRepo(win::AssetRoll &roll)
	: roll(roll)
{}

SoundRepo::~SoundRepo()
{
	if (entries.size() != 0)
		win::bug("Leftover sound entries");
}

Sound &SoundRepo::load(const char *name, bool cache, int seek)
{
	SoundRepoCacheEntry *cached = NULL;
	// create a cache entry if one doesn't exist yet
	for (auto &entry : cache_entries)
	{
		if (entry.key_name == name && entry.key_seek == seek)
		{
			cached = &entry;
			break;
		}
	}

	if (cached == NULL)
		cached = &cache_entries.add(name, seek);

	// load a decoder
	DecodingPcmSource &decoder = decoders.add(roll[name], seek, cached->channels, true);

	// make a sound object
	Sound &sound = entries.add(decoder, cached);
	return sound;
}

void SoundRepo::unload(Sound &sound)
{
	auto &entry = (SoundRepoEntry&)sound;

	DecodingPcmSource &source = (DecodingPcmSource&)sound.source;

	// fill in the cache information, if appropriate
	if (entry.cache_entry->channels == -1)
		entry.cache_entry->channels = sound.source.channels();

	entries.remove(entry);
	decoders.remove(source);
}

}
