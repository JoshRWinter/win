#ifndef WIN_APACK_H
#define WIN_APACK_H

#include <thread>
#include <memory>
#include <atomic>

namespace win
{

struct apack_sound
{
	std::atomic<unsigned long long> size;
	std::unique_ptr<short[]> buffer;
	std::unique_ptr<unsigned char[]> encoded;
	unsigned long long target_size;
	std::thread thread;
};

class audio_engine;
struct apack_remote;
class apack
{
	friend audio_engine;

public:
	apack() = default;
	apack(const data_list&);
	apack(apack&) = delete;
	apack(apack&&);
	~apack();

	void operator=(const apack&) = delete;
	apack &operator=(apack&&);

private:
	void finalize();

	std::unique_ptr<apack_remote> remote;
};

struct apack_remote
{
	apack_remote() = default;
	apack_remote(const apack_remote&) = delete;
	apack_remote(apack_remote&&) = delete;
	void operator=(const apack_remote&) = delete;
	void operator=(apack_remote&&) = delete;

	int count_;
	std::unique_ptr<apack_sound[]> stored_;
};

}

#endif
