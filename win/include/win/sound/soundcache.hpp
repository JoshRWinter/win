#ifndef WIN_SOUND_CACHE_HPP
#define WIN_SOUND_CACHE_HPP

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/sound/sound.hpp>
#include <win/sound/pcmresource.hpp>
#include <win/pool.hpp>

namespace win
{

class SoundCache
{
	WIN_NO_COPY_MOVE(SoundCache);

public:
	explicit SoundCache(win::AssetRoll&);
	~SoundCache();

	Sound &load(const char*, int);
	void unload(Sound&);

private:
	win::AssetRoll &roll;
	win::Pool<win::Sound, 25> loaded_sounds;
	win::Pool<PCMResource, 25> resources;
	win::Pool<PCMResource, 8> resources_staging;
};

}

#endif
