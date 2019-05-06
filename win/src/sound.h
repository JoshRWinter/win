#ifndef WIN_APACK_H
#define WIN_APACK_H

#include <thread>
#include <memory>
#include <atomic>

namespace win
{

class audio_engine;
struct sound_remote;
class sound
{
	friend audio_engine;

public:
	sound() = default;
	sound(win::data);
	sound(sound&) = delete;
	sound(sound&&);
	~sound();

	void operator=(const sound&) = delete;
	sound &operator=(sound&&);
	sound &operator=(win::data);

private:
	void from_roll(data&);
	void finalize();

	std::unique_ptr<sound_remote> remote;
};

struct sound_remote
{
	sound_remote() = default;
	sound_remote(const sound_remote&) = delete;
	sound_remote(sound_remote&&) = delete;
	void operator=(const sound_remote&) = delete;
	void operator=(sound_remote&&) = delete;

	std::atomic<unsigned long long> size;
	std::unique_ptr<short[]> buffer;
	std::unique_ptr<unsigned char[]> encoded;
	unsigned long long target_size;
	std::thread thread;
};

}

#endif
