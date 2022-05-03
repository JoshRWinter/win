#ifndef WIN_SOUNDENGINE_LINUX_HPP
#define WIN_SOUNDENGINE_LINUX_HPP

#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <win/assetroll.hpp>
#include <win/soundengine.hpp>
#include <win/soundenginecommon.hpp>

namespace win
{

class SoundEngineLinuxProxy
{
public:
	virtual ~SoundEngineLinuxProxy() = 0;

	virtual void process() = 0;

	virtual int play(const char*, bool = false) = 0; // ambient
	virtual int play(const char*, float, float, bool = false) = 0; // stereo
	virtual int play(const char*, float, float, bool, bool) = 0; // fully dressed function
	virtual void pause(int) = 0;
	virtual void resume(int) = 0;
	virtual void stop(int) = 0;
	virtual void source(int, float, float) = 0;
	virtual void listener(float, float) = 0;
};

inline SoundEngineLinuxProxy::~SoundEngineLinuxProxy() {}

class Display;
class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

public:
	SoundEngine(Display&, AssetRoll&, SoundConfigFn);
	~SoundEngine();

	void process() { inner->process(); }

	int play(const char *path, bool looping = false) { return inner->play(path, looping); }
	int play(const char *path, float x, float y, bool looping = false) { return inner->play(path, x, y, looping); }
	int play(const char *path, float x, float y, bool looping, bool ambient) { return inner->play(path, x, y, looping, ambient); }
	void pause(int sid) { inner->pause(sid); }
	void resume(int sid) { inner->resume(sid); }
	void stop(int sid) { inner->stop(sid); }
	void source(int sid, float x, float y) { inner->source(sid, x, y); }
	void listener(float x, float y) { inner->listener(x, y); }

private:
	std::unique_ptr<SoundEngineLinuxProxy> inner;
	void *so;
};

}

#endif

#endif
