#pragma once

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/sound/Sound.hpp>
#include <win/sound/PcmResource.hpp>
#include <win/Pool.hpp>

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
	win::Pool<PcmResource, 25> resources;
	win::Pool<PcmResource, 8> resources_staging;
};

}
