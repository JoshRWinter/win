#ifndef WIN_SOUND_MIXER_HPP
#define WIN_SOUND_MIXER_HPP

#include <memory>
#include <chrono>
#include <array>

#include <win/win.hpp>
#include <win/sound/activesoundstore.hpp>
#include <win/sound/pcmstream.hpp>
#include <win/sound/soundresidencypriority.hpp>
#include <win/sound/soundcache.hpp>
#include <win/assetroll.hpp>

namespace win
{

struct SoundMixerSound
{
	SoundMixerSound(Sound &sound, SoundResidencyPriority residency_priority, float compression_priority, float left, float right, bool looping)
		: sound(sound)
		, residency_priority(residency_priority)
		, compression_priority(compression_priority)
		, left(left)
		, right(right)
		, left_limiter(1.0f)
		, right_limiter(1.0f)
		, looping(looping)
		, playing(true)
		, done(false)
	{}

	Sound &sound;
	SoundResidencyPriority residency_priority;
	float compression_priority;
	float left;
	float right;
	float left_limiter;
	float right_limiter;
	bool looping;
	bool playing;
	bool done;
};

struct StereoLimiter { float *left, *right; };

class SoundMixer
{
	WIN_NO_COPY_MOVE(SoundMixer);

	static constexpr int max_sounds = 32;
	static constexpr int mix_samples = 360;
public:
	explicit SoundMixer(win::AssetRoll&);
	~SoundMixer();

	int add(const char*, win::SoundResidencyPriority, float, float, float, bool, int);
	void config(std::uint32_t, float, float);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void stop(std::uint32_t);
	void cleanup(bool);
	int mix_stereo(std::int16_t*, int);

private:
	void calculate_stereo_limiters(int, int, const std::array<StereoLimiter, max_sounds>&, const std::array<float, max_sounds>&);
	static void extract_stereo_f32(SoundMixerSound&, float*, int);

	std::unique_ptr<float[]> conversion_buffers_owner;
	float *conversion_buffers;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_call;
	win::SoundCache cache;
	win::ActiveSoundStore<SoundMixerSound, max_sounds> sounds;
};

}

#endif
