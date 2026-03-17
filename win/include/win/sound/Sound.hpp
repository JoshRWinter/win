#pragma once

#include <win/sound/PcmSource.hpp>
#include <win/Win.hpp>

namespace win
{

class Sound
{
	WIN_NO_COPY_MOVE(Sound);

public:
	explicit Sound(PcmSource &source)
		: source(source)
	{
	}

	win::PcmSource &source;
};

}
