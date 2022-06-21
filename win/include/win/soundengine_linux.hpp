#ifndef WIN_SOUNDENGINE_LINUX_HPP
#define WIN_SOUNDENGINE_LINUX_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <win/assetroll.hpp>
#include <win/soundengine.hpp>
#include <win/soundpriority.hpp>

namespace win
{

class SoundEngineLinuxProxy
{
public:
	virtual ~SoundEngineLinuxProxy() = 0;

	virtual std::uint32_t play(win::SoundPriority, const char*, bool = false) = 0;
	virtual std::uint32_t play(win::SoundPriority, const char*, float, float, bool = false) = 0;
	virtual void pause(std::uint32_t) = 0;
	virtual void resume(std::uint32_t) = 0;
	virtual void config(std::uint32_t, float, float) = 0;
};

inline SoundEngineLinuxProxy::~SoundEngineLinuxProxy() {}

class Display;
class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

public:
	SoundEngine(AssetRoll&);
	~SoundEngine();

	std::uint32_t play(win::SoundPriority priority, const char *path, bool looping = false) { return inner->play(priority, path, looping); }
	std::uint32_t play(win::SoundPriority priority, const char *path, float left, float right, bool looping = false) { return inner->play(priority, path, left, right, looping); }
	void pause(std::uint32_t sid) { inner->pause(sid); }
	void resume(std::uint32_t sid) { inner->resume(sid); }
	void config(std::uint32_t sid, float pan, float volume) { inner->config(sid, pan, volume); }

private:
	std::unique_ptr<SoundEngineLinuxProxy> inner;
	void *so;
};

}

#endif

#endif
