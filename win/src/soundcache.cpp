#include <win/soundcache.hpp>

namespace win
{

SoundCache::SoundCache(win::AssetRoll &roll)
	: roll(roll)
{}

SoundCache::~SoundCache()
{
	if (active_streams.size() != 0)
		win::bug("Leftover sound streams");
}

SoundStream &SoundCache::load_stream(const char *name)
{
    win::Stream s = roll[name];
    return active_streams.add(this, &s, SoundStreamKind::not_cached);
}

void SoundCache::unload_stream(SoundStream &stream)
{
	active_streams.remove(stream);
}

}
