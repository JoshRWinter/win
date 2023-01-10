#pragma once

#include <win/sound/floatpcmprovider.hpp>

namespace win
{

class SoundEffect : public FloatPCMProvider
{
	WIN_NO_COPY_MOVE(SoundEffect);

public:
	SoundEffect(int priority)
		: removed(false)
		, priority(priority)
		, inner(NULL)
		, prev(NULL)
	{}

	bool removed;
	int priority;
	FloatPCMProvider *inner;
	SoundEffect *prev;

protected:
	~SoundEffect() = default;
};

}
