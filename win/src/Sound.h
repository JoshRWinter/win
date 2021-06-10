#ifndef WIN_APACK_H
#define WIN_APACK_H

#include <thread>
#include <memory>
#include <atomic>

namespace win
{

class audio_engine;
class Sound
{
	friend class AudioEngine;
public:
	Sound(AssetRollStream);
	Sound(Sound&) = delete;
	Sound(Sound&&) = delete;
	~Sound();

	void operator=(const Sound&) = delete;
	Sound &operator=(Sound&&) = delete;
	Sound &operator=(AssetRollStream&);

private:
	void from_roll(AssetRollStream&);

	std::atomic<unsigned long long> size;
	std::unique_ptr<short[]> buffer;
	std::unique_ptr<unsigned char[]> encoded;
	unsigned long long target_size;
	std::thread thread;
};

}

#endif
