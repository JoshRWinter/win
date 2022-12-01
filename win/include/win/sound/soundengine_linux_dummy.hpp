#ifndef WIN_SOUNDENGINE_LINUX_DUMMY_HPP
#define WIN_SOUNDENGINE_LINUX_DUMMY_HPP

#include <win/win.hpp>
#include <win/sound/soundengine_linux.hpp>

namespace win
{

class SoundEngineLinuxDummy : public SoundEngineLinuxProxy
{
	WIN_NO_COPY_MOVE(SoundEngineLinuxDummy);

public:
	SoundEngineLinuxDummy() {}

	std::uint32_t play(const char*, int, float, bool, int) override { return -1; }
	std::uint32_t play(const char*, int, float, float, float, bool, int) override { return -1; }
	void apply_effect(std::uint32_t, SoundEffect*) override {}
	void remove_effect(std::uint32_t, SoundEffect*) override {}
	void pause(std::uint32_t) override {}
	void resume(std::uint32_t) override {}
	void stop(std::uint32_t) override {}
	void config(std::uint32_t, float, float) override {}
};

}

#endif
