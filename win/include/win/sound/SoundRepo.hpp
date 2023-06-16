#pragma once

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/Pool.hpp>
#include <win/sound/Sound.hpp>
#include <win/sound/PcmSource.hpp>
#include <win/sound/DecodingPcmSource.hpp>

namespace win
{

class SoundRepo
{
	WIN_NO_COPY_MOVE(SoundRepo);

public:
	explicit SoundRepo(win::AssetRoll&);
	~SoundRepo();

	Sound &load(const char*, int);
	void unload(Sound&);

private:
	AssetRoll &roll;
	Pool<Sound, 25> loaded_sounds;
	Pool<DecodingPcmSource, 25> decoders;
};

}
