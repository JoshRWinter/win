#include <win/sound/SoundRepo.hpp>

namespace win
{

SoundRepo::SoundRepo(win::AssetRoll &roll)
	: roll(roll)
{}

SoundRepo::~SoundRepo()
{
	if (sounds.size() != 0)
		win::bug("Leftover sound streams");
}

Sound &SoundRepo::load(const char *name, bool cache, int seek)
{
	// load a decoder
	DecodingPcmSource &source = decoders.add(roll[name], seek);

	// make a sound object
	Sound &sound = sounds.add(&source);
	return sound;
}

void SoundRepo::unload(Sound &sound)
{
	DecodingPcmSource &source = *(DecodingPcmSource*)sound.source;
	sounds.remove(sound);

	decoders.remove(source);
}

}
