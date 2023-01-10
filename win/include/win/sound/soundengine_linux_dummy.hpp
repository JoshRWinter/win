#pragma once

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <win/sound/soundengine.hpp>

namespace win
{

class SoundEngineLinuxDummy : public SoundEngineImplementation
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
