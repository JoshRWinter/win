#ifndef WIN_SOUND_CACHE_HPP
#define WIN_SOUND_CACHE_HPP

#include <array>

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/soundstream.hpp>
#include <win/pool.hpp>

namespace win
{

class Sound;
class SoundCache
{
	friend class Sound;
	WIN_NO_COPY_MOVE(SoundCache);

public:
	static constexpr int pcm_chunk_cache_size = 44100 * 5; // 5 seconds of mono, or 2.5 seconds of stereo
	SoundCache(win::AssetRoll&);
	~SoundCache();

	SoundStream &load_stream(const char*);
	void unload_stream(SoundStream&);

private:
	win::AssetRoll &roll;
	win::Pool<SoundStream> active_streams;
	//win::Pool<std::string> name_cache;
	//win::Pool<std::array<std::int16_t, pcm_chunk_cache_size>> pcm_chunk_cache;
};

}

#endif
