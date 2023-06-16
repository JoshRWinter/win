#pragma once

#include <memory>
#include <chrono>
#include <array>

#include <win/Win.hpp>
#include <win/sound/ActiveSoundStore.hpp>
#include <win/sound/SoundRepo.hpp>
#include <win/AssetRoll.hpp>

namespace win
{

struct SoundMixerSound
{
	SoundMixerSound(Sound &sound, int residency_priority, float compression_priority, float left, float right, bool looping)
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
	int residency_priority;
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

public:
	static constexpr int max_sounds = 32;
	static constexpr int mix_samples = 360;

	explicit SoundMixer(win::AssetRoll &roll);
	~SoundMixer();

	std::uint32_t add(const char *name, int residency_priority, float compression_priority, float left, float right, bool loop, bool cache, int seek);
	void config(std::uint32_t key, float left, float right);
	void pause(std::uint32_t key);
	void resume(std::uint32_t key);
	void stop(std::uint32_t key);
	void cleanup(bool all);
	int mix_stereo(std::int16_t *dest, int len);

private:
	void calculate_stereo_limiters(int count, int len, const std::array<StereoLimiter, max_sounds> &limiters, const std::array<float, max_sounds> &priorities);
	static void extract_stereo_f32(SoundMixerSound&, float *dest, int len);
	static void zero_float(float*, int len);

	std::unique_ptr<float[]> conversion_buffers_owner;
	float *conversion_buffers;
	std::chrono::time_point<std::chrono::high_resolution_clock> last_call;
	win::SoundRepo repo;
	win::ActiveSoundStore<SoundMixerSound, max_sounds> sounds;
};

}
