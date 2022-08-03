#ifndef WIN_SOUND_HPP
#define WIN_SOUND_HPP

#include <win/win.hpp>
#include <win/stream.hpp>
#include <win/pcmstream.hpp>
#include <win/pcmdecoder.hpp>

namespace win
{

class Sound
{
	WIN_NO_COPY_MOVE(Sound);

public:
	Sound(PCMResource &resource, win::Stream *datafile, int seek_start)
		: source(stream, resource, datafile, seek_start)
	{}

	win::PCMStream stream;
	win::PCMDecoder source;
};

}

#endif
