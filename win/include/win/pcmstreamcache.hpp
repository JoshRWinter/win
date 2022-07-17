#ifndef WIN_SOUND_CACHE_HPP
#define WIN_SOUND_CACHE_HPP

#include <array>

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/pcmstream.hpp>
#include <win/pool.hpp>

namespace win
{

struct CachedPCMStream
{
	static constexpr int pcm_cache_len = 44100 * 5; // 5 seconds of mono, or 2.5 seconds of stereo

	CachedPCMStream(const char *name)
		: name(name)
		, fully_cached(false)
		, cache_fill(0)
		, completed(false)
	{}

	void complete(bool fully_cached, int cache_fill)
	{
		this->fully_cached = fully_cached;
		this->cache_fill = cache_fill;
		completed = true;
	}

	const std::string name;
    bool fully_cached;
	int cache_fill;
	std::array<std::int16_t, pcm_cache_len> pcm;
	bool completed;
};

class PCMStreamCache
{
	WIN_NO_COPY_MOVE(PCMStreamCache);

public:
	PCMStreamCache(win::AssetRoll&);
	~PCMStreamCache();

	PCMStream &load_stream(const char*);
	void unload_stream(PCMStream&);

private:
	win::AssetRoll &roll;
	win::Pool<PCMStream, 25> loaded_streams;
	win::Pool<CachedPCMStream, 25> cached_streams;
};

}

#endif
