#ifndef WIN_SOUND_CACHE_HPP
#define WIN_SOUND_CACHE_HPP

#include <array>

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/pcmstream.hpp>
#include <win/pool.hpp>

namespace win
{

class PCMStreamCache
{
	WIN_NO_COPY_MOVE(PCMStreamCache);

public:
	static constexpr int pcm_chunk_cache_size = 44100 * 5; // 5 seconds of mono, or 2.5 seconds of stereo
	PCMStreamCache(win::AssetRoll&);
	~PCMStreamCache();

	PCMStream &load_stream(const char*);
	void unload_stream(PCMStream&);

private:
	win::AssetRoll &roll;
	win::Pool<PCMStream> loaded_streams;
	//win::Pool<std::string> name_cache;
	//win::Pool<std::array<std::int16_t, pcm_chunk_cache_size>> pcm_chunk_cache;
};

}

#endif
