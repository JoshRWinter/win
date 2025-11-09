#pragma once

#include <vector>
#include <memory>

#include <win/Win.hpp>
#include <win/AssetRoll.hpp>
#include <win/Display.hpp>

namespace win
{

struct SoundEnginePlayCommand
{
	SoundEnginePlayCommand(const char *name, int residency_priority, float compression_priority, float left, float right, bool looping, bool cache, int seek)
		: name(name)
		, residency_priority(residency_priority)
		, compression_priority(compression_priority)
		, left(left)
		, right(right)
		, looping(looping)
		, cache(cache)
		, seek(seek)
	{}

	const char *name;
	int residency_priority;
	float compression_priority;
	float left;
	float right;
	bool looping;
	bool cache;
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

class SoundEngineBase
{
	WIN_NO_COPY_MOVE(SoundEngineBase);

public:
	SoundEngineBase() = default;

	virtual std::uint32_t play(const SoundEnginePlayCommand &cmd) = 0;
	virtual void save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &configs) = 0;

	virtual ~SoundEngineBase() = default;
};

#if defined WINPLAT_WINDOWS
typedef HWND WindowHandle;
#elif defined WINPLAT_LINUX
typedef void* window_handle;
#endif

class SoundEngine
{
	WIN_NO_COPY_MOVE(SoundEngine);

public:
	SoundEngine(AssetRoll&);

	// name must live until the save() call returns
	std::uint32_t play(const char *name, int residency_priority, float compression_priority, float left, float right, bool looping, bool cache, int seek = 0);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void stop(std::uint32_t);
	void config(std::uint32_t, float, float);

	// save config and playback commands
	void save();

private:
	std::unique_ptr<SoundEngineBase> inner;

	std::vector<SoundEnginePlayCommand> play_commands;
	std::vector<SoundEnginePlaybackCommand> playback_commands;
	std::vector<SoundEngineConfigCommand> config_commands;
};

}
