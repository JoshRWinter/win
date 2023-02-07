#pragma once

#include <win/Win.hpp>
#include <win/Stream.hpp>
#include <win/sound/PcmStream.hpp>
#include <win/sound/PcmDecoder.hpp>

namespace win
{

class Sound
{
	WIN_NO_COPY_MOVE(Sound);

public:
	Sound(PcmResource &resource, win::Stream *datafile, int seek_start)
		: source(stream, resource, datafile, seek_start)
	{}

	~Sound()
	{
		source.stop();
	}

	win::PcmStream stream;
	win::PcmDecoder source;
};

}
