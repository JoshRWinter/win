#ifndef WIN_APACK_H
#define WIN_APACK_H

#include <thread>
#include <atomic>

namespace win
{

class apack;

struct apack_sound
{
	apack *parent;
	std::atomic<unsigned long long> size;
	std::unique_ptr<short[]> buffer;
	std::unique_ptr<unsigned char[]> encoded;
	unsigned long long target_size;
	std::thread thread;
};

class apack
{
public:
	apack(resource&);
	apack(const apack&) = delete;
	apack(apack&&);
	~apack();

	void operator=(const apack&) = delete;
	apack &operator=(apack&&);

private:
	void finalize();

	std::unique_ptr<apack_sound[]> sounds_;
	std::uint8_t count_;
};

}

#endif
