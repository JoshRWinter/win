#pragma once

#include <vector>
#include <memory>

#include <win/win.hpp>
#include <win/assetroll.hpp>
#include <win/sound/soundeffect.hpp>
#include <win/display.hpp>

namespace win
{

struct SoundEnginePlayCommand
{
	SoundEnginePlayCommand(const char *name, int residency_priority, float compression_priority, float left, float right, bool looping, int seek)
		: name(name)
		, residency_priority(residency_priority)
		, compression_priority(compression_priority)
		, left(left)
		, right(right)
		, looping(looping)
		, seek(seek)
	{}

	const char *name;
	int residency_priority;
	float compression_priority;
	float left;
	float right;
	bool looping;
	int seek;
};

struct SoundEnginePlaybackCommand
{
	SoundEnginePlaybackCommand(std::uint32_t key, bool playing, bool stop)
		: key(key)
		, playing(playing)
		, stop(stop)
	{}

	std::uint32_t key;
	bool playing;
	bool stop;
};

struct SoundEngineConfigCommand
{
	SoundEngineConfigCommand(std::uint32_t key, float left, float right)
		: key(key)
		, left(left)
		, right(right)
	{}

	std::uint32_t key;
	float left;
	float right;
};

class SoundEngineImplementation
{
	WIN_NO_COPY_MOVE(SoundEngineImplementation);

public:
	SoundEngineImplementation() = default;

	virtual std::uint32_t play(const SoundEnginePlayCommand&) = 0;
	virtual void save(const std::vector<SoundEnginePlaybackCommand>&, const std::vector<SoundEngineConfigCommand>&) = 0;

	virtual ~SoundEngineImplementation() {}
};

#if defined WINPLAT_WINDOWS
typedef HWND WindowHandle;
#elif defined WINPLAT_LINUX
typedef void* window_handle;
#endif

class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

	SoundEngine(Display*, AssetRoll&);
public:
	SoundEngine(Display&, AssetRoll&);
	SoundEngine(AssetRoll&);

	// name must live until the save() call returns
	std::uint32_t play(const char *name, int, float, bool = false, int = 0);
	// name must live until the save() call returns
	std::uint32_t play(const char*, int, float, float, float, bool, int = 0);
	//void apply_effect(std::uint32_t, SoundEffect*);
	//void remove_effect(std::uint32_t, SoundEffect*);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void stop(std::uint32_t);
	void config(std::uint32_t, float, float);

	// save config and playback commands
	void save();

private:
	std::unique_ptr<SoundEngineImplementation> inner;

	std::vector<SoundEnginePlayCommand> play_commands;
	std::vector<SoundEnginePlaybackCommand> playback_commands;
	std::vector<SoundEngineConfigCommand> config_commands;
};

}
