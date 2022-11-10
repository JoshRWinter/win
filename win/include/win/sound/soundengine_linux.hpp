#ifndef WIN_SOUNDENGINE_LINUX_HPP
#define WIN_SOUNDENGINE_LINUX_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <win/assetroll.hpp>
#include <win/sound/soundresidencypriority.hpp>
#include <win/sound/soundeffect.hpp>

namespace win
{

class SoundEngineLinuxProxy
{
public:
	virtual ~SoundEngineLinuxProxy();

	virtual std::uint32_t play(const char*, win::SoundResidencyPriority, float, bool, int) = 0;
	virtual std::uint32_t play(const char*, win::SoundResidencyPriority, float, float, float, bool, int) = 0;
	virtual void apply_effect(std::uint32_t, SoundEffect*) = 0;
	virtual void remove_effect(std::uint32_t, SoundEffect*) = 0;
	virtual void pause(std::uint32_t) = 0;
	virtual void resume(std::uint32_t) = 0;
	virtual void stop(std::uint32_t) = 0;
	virtual void config(std::uint32_t, float, float) = 0;
};

inline SoundEngineLinuxProxy::~SoundEngineLinuxProxy() {}

class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

public:
	SoundEngine(AssetRoll&);
	~SoundEngine();

	std::uint32_t play(const char *path, win::SoundResidencyPriority priority, float compression_priority, bool looping = false, int seek = 0) { return inner->play(path, priority, compression_priority, looping, seek); }
	std::uint32_t play(const char *path, win::SoundResidencyPriority priority, float compression_priority, float left, float right, bool looping = false, int seek = 0) { return inner->play(path, priority, compression_priority, left, right, looping, seek); }
	void apply_effect(std::uint32_t sid, SoundEffect *effect) { inner->apply_effect(sid, effect); }
	void remove_effect(std::uint32_t sid, SoundEffect *effect) { inner->remove_effect(sid, effect); }
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
