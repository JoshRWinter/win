#ifndef WIN_APACK_HPP
#define WIN_APACK_HPP

#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <list>

#include <win.h>

#if defined WINPLAT_LINUX
#include <pulse/pulseaudio.h>
#elif defined WINPLAT_WINDOWS
#include <dsound.h>
#endif

namespace win
{
class Sound;
}

void decodeogg(win::Stream source, win::Sound *sound, std::atomic<bool> *cancel);

namespace win
{

struct SoundPage
{
	SoundPage(bool = false);
	SoundPage(std::int16_t*, unsigned long long);
	SoundPage(const SoundPage&) = delete;
	SoundPage(SoundPage&&) = delete;

	void operator=(const SoundPage&) = delete;
	void operator=(SoundPage&&) = delete;

	static constexpr int PAGE_SAMPLE_COUNT = 44'100 * 4;

	bool cache_this_page;
	bool read_only;
	std::int16_t *samples;
	std::unique_ptr<std::int16_t[]> samples_owner;
	std::atomic<unsigned long long> samples_filled;
	std::atomic<unsigned long long> fake_samples_filled;
	std::atomic<unsigned long long> samples_consumed;
};

class SoundBank;
class SoundEngine;
class Sound
{
	friend class SoundEngine;
	friend void ::decodeogg(win::Stream, win::Sound*, std::atomic<bool>*);
public:
	static constexpr int MAX_PAGES = 3;

	Sound(Stream&&, const std::string&, SoundBank*);
	Sound(const std::string &name, Stream*, int, std::int16_t*, unsigned long long);
	Sound(Sound&) = delete;
	Sound(Sound&&) = delete;
	~Sound();

	void operator=(const Sound&) = delete;
	Sound &operator=(Sound&&) = delete;

	unsigned long long write(const std::int16_t*, unsigned long long);
	void complete_writing();
	unsigned long long read(std::int16_t*, unsigned long long);
	bool is_stream_completed();

private:
	unsigned long long fake_write(const std::int16_t*, unsigned long long, SoundPage*);

	std::atomic<int> channels;
	SoundBank *parent;
	std::string name;
	std::thread decoder_worker;
	std::atomic<bool> decoder_worker_cancel;
	std::list<SoundPage> pages;
	std::mutex pages_lock;
	std::atomic<bool> writing_completed;
};

}

#endif
