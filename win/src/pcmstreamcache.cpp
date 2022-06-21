#include <win/pcmstreamcache.hpp>

namespace win
{

PCMStreamCache::PCMStreamCache(win::AssetRoll &roll)
	: roll(roll)
{}

PCMStreamCache::~PCMStreamCache()
{
	if (loaded_streams.size() != 0)
		win::bug("Leftover sound streams");
}

PCMStream &PCMStreamCache::load_stream(const char *name)
{
	win::Stream s = roll[name];
	// the win::SoundStream will std::move the "s" into itself
	return loaded_streams.add(this, &s, PCMStreamCacheMode::not_cached);
}

void PCMStreamCache::unload_stream(PCMStream &stream)
{
	loaded_streams.remove(stream);
}

}
