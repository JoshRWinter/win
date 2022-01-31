#include <win.h>

namespace win
{

SoundBank::SoundBank(AssetRoll &roll)
	: roll(roll)
{
}

SoundBank::~SoundBank()
{
	if(sounds.size() != 0)
		win::bug("leftover sounds");
}

Sound &SoundBank::load(const char *name)
{
	CachedSound *cached_sound = NULL;
	for(auto &sound : cached_sounds)
	{
		if(sound.name == name)
		{
			cached_sound = &sound;
			break;
		}
	}

	if(cached_sound != NULL)
	{
		const bool need_to_decode = cached_sound->samples_count >= SoundPage::PAGE_SAMPLE_COUNT; // good enough heuristic
		if(need_to_decode)
		{
			Stream stream = roll[name];
			auto &sound = sounds.emplace_back(cached_sound->name, &stream, cached_sound->channels, cached_sound->samples.get(), cached_sound->samples_count);
			return sound;
		}
		else
		{
			auto &sound = sounds.emplace_back(cached_sound->name, (Stream*)NULL, cached_sound->channels, cached_sound->samples.get(), cached_sound->samples_count);
			return sound;
		}
	}
	else
	{
		auto &sound = sounds.emplace_back(roll[name], name, this);
		return sound;
	}
}

void SoundBank::unload(Sound &sound)
{
	for(auto it = sounds.begin(); it != sounds.end(); ++it)
	{
		if(&sound == &(*it))
		{
			sounds.erase(it);
			return;
		}
	}

	win::bug("could not find sound");
}

bool SoundBank::cache_sound(const std::string &name, int channels, std::int16_t *samples, unsigned long long samples_count)
{
	bool found = false;
	for(const auto &sound : cached_sounds)
	{
		if (sound.name == name)
		{
			found = true;
		}
	}

	if(found)
		return false;

	cached_sounds.emplace_back(std::move(name), channels, samples, samples_count);
	return true;
}

}
