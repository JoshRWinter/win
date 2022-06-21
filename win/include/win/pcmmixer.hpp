#ifndef WIN_PCMMIXER_HPP
#define WIN_PCMMIXER_HPP

#include <win/win.hpp>
#include <win/activesoundstore.hpp>
#include <win/pcmstream.hpp>
#include <win/pcmstreamcache.hpp>
#include <win/assetroll.hpp>

namespace win
{

struct PCMMixerStream
{
	PCMMixerStream(SoundPriority priority, PCMStream *sound, float left, float right, bool looping)
		: priority(priority)
		, sound(sound)
		, left(left)
		, right(right)
		, looping(looping)
	{}

	SoundPriority priority;
	PCMStream *sound;
	float left;
	float right;
	bool looping;
};

class PCMMixer
{
	WIN_NO_COPY_MOVE(PCMMixer);

public:
	PCMMixer(win::AssetRoll&);
	int add(win::SoundPriority, const char*, float, float, bool);
	void config(std::uint32_t, float, float);
	void remove(std::uint32_t);
	int mix_samples(std::int16_t*, int);

private:
	win::PCMStreamCache cache;
	win::ActiveSoundStore<PCMMixerStream, 32> streams;
};

}

#endif
