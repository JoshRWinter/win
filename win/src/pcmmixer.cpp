#include <win/pcmmixer.hpp>

namespace win
{

PCMMixer::PCMMixer(win::AssetRoll &roll)
	: cache(roll)
{

}

int PCMMixer::add(win::SoundPriority priority, const char *name, float left, float right, bool looping)
{
	const 1
	const auto key = streams.add(priority, name, left, right, looping);
	return key;
}

void PCMMixer::config(std::uint32_t key, float left, float right)
{

}

void PCMMixer::remove(std::uint32_t key)
{

}

int PCMMixer::mix_samples(std::int16_t *dest, int len)
{
	memset(dest, 0, len * sizeof(std::int16_t));

	if (streams.size() == 0)
		return len;
	else
	{
		PCMStream &s = *(streams.begin()->sound);
		return s.read_samples(dest, len);
	}
}

}
