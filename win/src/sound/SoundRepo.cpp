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
	// load a decoder
	DecodingPcmSource &source = decoders.add(roll[name], seek);

	// make a sound object
	Sound &sound = loaded_sounds.add(&source);
	return sound;
}

void SoundRepo::unload(Sound &sound)
{
	DecodingPcmSource &source = *(DecodingPcmSource*)sound.source;
	loaded_sounds.remove(sound);

	decoders.remove(source);
}

}
