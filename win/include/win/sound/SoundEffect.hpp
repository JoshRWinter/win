#pragma once

#include <win/sound/FloatPcmProvider.hpp>

namespace win
{

class SoundEffect : public FloatPcmProvider
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
	FloatPcmProvider *inner;
	SoundEffect *prev;

protected:
	~SoundEffect() = default;
};

}
