#pragma once

#include <win/Win.hpp>
#include <win/sound/PcmSource.hpp>

namespace win
{

class Sound
{
	WIN_NO_COPY_MOVE(Sound);

public:
	explicit Sound(PcmSource &source)
		: source(source)
	{}

	win::PcmSource &source;
};

}
