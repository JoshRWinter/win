#ifndef WIN_PCMMIXER_HPP
#define WIN_PCMMIXER_HPP

#include <memory>

#include <win/win.hpp>
#include <win/activesoundstore.hpp>
#include <win/pcmstream.hpp>
#include <win/pcmstreamcache.hpp>
#include <win/assetroll.hpp>

namespace win
{

struct PCMMixerStream
{
	PCMMixerStream(PCMStream &stream, float left, float right, bool looping)
		: stream(stream)
		, left(left)
		, right(right)
		, looping(looping)
		, playing(true)
		, done()
	{}

	PCMStream &stream;
	float left;
	float right;
	bool looping;
	bool playing;
	bool done;
};

class PCMMixer
{
	WIN_NO_COPY_MOVE(PCMMixer);

	static constexpr int max_streams = 32;
    static constexpr int mix_samples = 360;
public:
	PCMMixer(win::AssetRoll&);
	int add(win::SoundPriority, const char*, float, float, bool);
	void config(std::uint32_t, float, float);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void remove(std::uint32_t);
	void cleanup(bool);
	int mix_stereo(std::int16_t*, int);

private:
	std::unique_ptr<std::int16_t[]> conversion_buffers_owner;
	std::int16_t *conversion_buffers;
	win::PCMStreamCache cache;
	win::ActiveSoundStore<PCMMixerStream, max_streams> streams;
};

}

#endif
