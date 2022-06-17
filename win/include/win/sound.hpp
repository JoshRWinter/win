#ifndef WIN_SOUND_HPP
#define WIN_SOUND_HPP

#include <win/soundcache.hpp>

namespace win
{

class Sound
{
public:
	Sound(SoundCache &cache, SoundStream &stream)
	: cache(cache)
	, stream(stream)
	{}

	~Sound()
	{
		cache.unload_stream(stream);
	}


	SoundStream &stream;

private:
	SoundCache &cache;
};

}

#endif
