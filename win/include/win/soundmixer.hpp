#ifndef WIN_SOUND_MIXER_HPP
#define WIN_SOUND_MIXER_HPP

#include <memory>

#include <win/win.hpp>
#include <win/activesoundstore.hpp>
#include <win/pcmstream.hpp>
#include <win/soundresidencypriority.hpp>
#include <win/soundcache.hpp>
#include <win/assetroll.hpp>

namespace win
{

struct SoundMixerSound
{
	SoundMixerSound(Sound &sound, SoundResidencyPriority residency_priority, float compression_priority, float left, float right, bool looping)
		: sound(sound)
		, residency_priority(residency_priority)
		, compression_priority(compression_priority)
		, left(left)
		, right(right)
		, looping(looping)
		, playing(true)
		, done(false)
	{}

	Sound &sound;
	SoundResidencyPriority residency_priority;
	float compression_priority;
	float left;
	float right;
	bool looping;
	bool playing;
	bool done;
};

class SoundMixer
{
	WIN_NO_COPY_MOVE(SoundMixer);

	static constexpr int max_sounds = 32;
    static constexpr int mix_samples = 360;
public:
	SoundMixer(win::AssetRoll&);
	~SoundMixer();

	int add(const char*, win::SoundResidencyPriority, float, float, float, bool, int);
	void config(std::uint32_t, float, float);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void remove(std::uint32_t);
	void cleanup(bool);
	int mix_stereo(std::int16_t*, int);

private:
	std::unique_ptr<std::int16_t[]> conversion_buffers_owner;
	std::int16_t *conversion_buffers;
	win::SoundCache cache;
	win::ActiveSoundStore<SoundMixerSound, max_sounds> sounds;
};

}

#endif
