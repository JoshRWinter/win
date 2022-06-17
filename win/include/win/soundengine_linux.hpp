#ifndef WIN_SOUNDENGINE_LINUX_HPP
#define WIN_SOUNDENGINE_LINUX_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <win/assetroll.hpp>
#include <win/soundengine.hpp>

namespace win
{

class SoundEngineLinuxProxy
{
public:
	virtual ~SoundEngineLinuxProxy() = 0;

	virtual std::uint32_t play(const char*, bool = false) = 0; // ambient
	virtual std::uint32_t play(const char*, float, float, bool = false) = 0; // stereo
	virtual std::uint32_t play(const char*, float, float, bool, bool) = 0; // fully dressed function
	virtual void pause(std::uint32_t) = 0;
	virtual void resume(std::uint32_t) = 0;
	virtual void stop(std::uint32_t) = 0;
	virtual void config(std::uint32_t, float, float) = 0;
};

inline SoundEngineLinuxProxy::~SoundEngineLinuxProxy() {}

class Display;
class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

public:
	SoundEngine(/*Display&, */AssetRoll&);
	~SoundEngine();

	std::uint32_t play(const char *path, bool looping = false) { return inner->play(path, looping); }
	std::uint32_t play(const char *path, float x, float y, bool looping = false) { return inner->play(path, x, y, looping); }
	std::uint32_t play(const char *path, float x, float y, bool looping, bool ambient) { return inner->play(path, x, y, looping, ambient); }
	void pause(std::uint32_t sid) { inner->pause(sid); }
	void resume(std::uint32_t sid) { inner->resume(sid); }
	void stop(std::uint32_t sid) { inner->stop(sid); }
	void config(std::uint32_t sid, float pan, float volume) { inner->config(sid, pan, volume); }

private:
	std::unique_ptr<SoundEngineLinuxProxy> inner;
	void *so;
};

}

#endif

#endif
