#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <win/sound/SoundEngine.hpp>

namespace win
{

class SoundEngineLinuxDummy : public SoundEngineBase
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxDummy);

public:
	SoundEngineLinuxDummy() = default;

	std::uint32_t play(const SoundEnginePlayCommand&) override
	{
		return -1;
	}

	void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) override
	{
	}
};

}

#endif
