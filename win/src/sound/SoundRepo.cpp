#include <win/sound/SoundRepo.hpp>

namespace win
{

SoundRepo::SoundRepo(win::AssetRoll &roll)
	: roll(roll)
{}

SoundRepo::~SoundRepo()
{
	if (loaded_sounds.size() != 0)
		win::bug("Leftover sound streams");
}

Sound &SoundRepo::load(const char *name, bool cache, int seek)
{
	SoundRepoCacheEntry *cached = NULL;
	// create a cache entry if one doesn't exist yet
	for (auto &entry : sound_cache)
	{
		if (entry.key_name == name && entry.key_seek == seek)
		{
			cached = &entry;
			break;
		}
	}

	if (cached == NULL)
		sound_cache.add(name, seek);

	int *cached_channels = NULL;
	if (cached && cached->channels != -1)
		cached_channels = &cached->channels;

	// load a decoder
	DecodingPcmSource &source = loaded_decoder_storage.add(roll[name], seek, cached_channels);

	// make a sound object
	Sound &sound = loaded_sounds.add(&source);
	return sound;
}

void SoundRepo::unload(Sound &sound)
{
	DecodingPcmSource &source = *(DecodingPcmSource*)sound.source;
	loaded_sounds.remove(sound);

	loaded_decoder_storage.remove(source);
}

}
