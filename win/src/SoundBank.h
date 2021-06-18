#ifndef WIN_SOUND_BANK_H
#define WIN_SOUND_BANK_H

namespace win
{

struct CachedSound
{
	CachedSound(const std::string &name, int channels, std::int16_t *samples, unsigned long long samples_count)
		: name(name), channels(channels), samples(samples), samples_count(samples_count) {}
	CachedSound(const CachedSound&) = delete;
	CachedSound(CachedSound&&) = delete;

	void operator=(const CachedSound&) = delete;
	void operator=(CachedSound&&) = delete;

	std::string name;
	std::unique_ptr<std::int16_t[]> samples;
	int channels;
	unsigned long long samples_count;
};

class SoundBank
{
	friend class Sound;
public:
	SoundBank(AssetRoll&);
	SoundBank(const SoundBank&) = delete;
	SoundBank(SoundBank&&) = delete;
	~SoundBank();

	void operator=(const SoundBank&) = delete;
	void operator=(SoundBank&&) = delete;

	Sound &load(const char*);
	void unload(Sound&);

private:
	bool cache_sound(const std::string&, int, std::int16_t*, unsigned long long);
	std::list<Sound> sounds;
	std::list<CachedSound> cached_sounds;
	AssetRoll &roll;
};

}

#endif
