#pragma once

#include <memory>
#include <chrono>
#include <array>

#include <win/Win.hpp>
#include <win/sound/ActiveSoundStore.hpp>
#include <win/sound/PcmStream.hpp>
#include <win/sound/SoundEffect.hpp>
#include <win/sound/SoundCache.hpp>
#include <win/AssetRoll.hpp>

namespace win
{

struct SoundMixerSound
{
	SoundMixerSound(Sound &sound, int residency_priority, float compression_priority, float left, float right, bool looping)
		: sound(sound)
		, residency_priority(residency_priority)
		, compression_priority(compression_priority)
		, effect_tail(NULL)
		, left(left)
		, right(right)
		, left_limiter(1.0f)
		, right_limiter(1.0f)
		, looping(looping)
		, playing(true)
		, done(false)
	{}

	Sound &sound;
	int residency_priority;
	float compression_priority;
	SoundEffect *effect_tail;
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

public:
	static constexpr int max_sounds = 32;
	static constexpr int mix_samples = 360;

	explicit SoundMixer(win::AssetRoll&);
	~SoundMixer();

	std::uint32_t add(const char*, int, float, float, float, bool, int);
	void config(std::uint32_t, float, float);
	void apply_effect(std::uint32_t, SoundEffect*);
	void remove_effect(std::uint32_t, SoundEffect*);
	void pause(std::uint32_t);
	void resume(std::uint32_t);
	void stop(std::uint32_t);
	void cleanup(bool);
	int mix_stereo(std::int16_t*, int);

private:
	void calculate_stereo_limiters(int, int, const std::array<StereoLimiter, max_sounds>&, const std::array<float, max_sounds>&);
	static void extract_stereo_f32(SoundMixerSound&, float*, int);
	static int extract_pcm(SoundMixerSound&, float*, int);
	static void zero_float(float*, int len);

	std::unique_ptr<float[]> conversion_buffers_owner;
	float *conversion_buffers;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_call;
	win::SoundCache cache;
	win::ActiveSoundStore<SoundMixerSound, max_sounds> sounds;
};

}
