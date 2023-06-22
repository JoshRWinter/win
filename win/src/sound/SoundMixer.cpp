#include <limits>
#include <cmath>
#include <algorithm>

#include <win/sound/SoundMixer.hpp>

namespace win
{

SoundMixer::SoundMixer(win::AssetRoll &roll)
	: repo(roll)
{
	static_assert(mix_samples % 2 == 0, "mix_samples must be divisible by 2");
	conversion_buffers_owner.reset(new float[max_sounds * mix_samples]);
	conversion_buffers = conversion_buffers_owner.get();

	last_call = std::chrono::high_resolution_clock::now();
}

SoundMixer::~SoundMixer()
{
	cleanup(true);
}

std::uint32_t SoundMixer::add(const char *name, int residency_priority, float compression_priority, float left, float right, bool looping, bool cache, int seek)
{
	cleanup(false);

	if (compression_priority > 0.99f)
		compression_priority = 0.99f;

	Sound &sound = repo.load(name, cache, seek);
	const auto key = sounds.add(sound, residency_priority, compression_priority, std::max(left, 0.0f), std::max(right, 0.0f), looping);
	if (key == -1)
	{
		// search the active sound store for one with less priority
		bool found = false;
		for (auto it = sounds.begin(); it != sounds.end(); ++it)
		{
			if (it->residency_priority < residency_priority)
			{
				repo.unload(it->sound);
				sounds.remove(it);
				found = true;
				break;
			}
		}

		if (!found)
		{
			// nevermind, need to unload it
			repo.unload(sound);
			return -1;
		}

		const auto new_key = sounds.add(sound, residency_priority, compression_priority, std::max(left, 0.0f), std::max(right, 0.0f), looping);

		if (new_key == -1)
			win::bug("SoundMixer: Couldn't add sound even after evicting one");
	}

	return key;
}

void SoundMixer::config(std::uint32_t key, float left, float right)
{
	SoundMixerSound *sound = sounds[key];
	if (sound == NULL)
		return;

	sound->left = left;
	sound->right = right;
}

void SoundMixer::pause(std::uint32_t key)
{
	SoundMixerSound *sound = sounds[key];
	if (sound == NULL)
		return;

	sound->playing = false;
}

void SoundMixer::resume(std::uint32_t key)
{
	SoundMixerSound *sound = sounds[key];
	if (sound == NULL)
		return;

	sound->playing = true;
}

void SoundMixer::cleanup(bool all)
{
	for (auto sound = sounds.begin(); sound != sounds.end();)
	{
		const bool kill = sound->done || all;

		if (kill)
		{
			repo.unload(sound->sound);
			sound = sounds.remove(sound);
			continue;
		}

		++sound;
	}
}

void SoundMixer::stop(std::uint32_t key)
{
	SoundMixerSound *sound = sounds[key];
	if (sound == NULL)
		return;

	sound->done = true;
}

// called by audio callback thread
// all other methods are called by "main" thread
int SoundMixer::mix_stereo(std::int16_t *dest, int len)
{
	const auto start = std::chrono::high_resolution_clock::now();

	len = std::min(len, mix_samples); // cap it
	if (len % 2 != 0) --len; // make sure it's even
	if (len <= 0) return 0;

	const float micros_since_last_call = std::chrono::duration<float, std::ratio<1, 1'000'000>>(start - last_call).count();
	last_call = start;

	std::array<StereoLimiter, max_sounds> limiters;
	std::array<float, max_sounds> priorities;

	// fill up the conv buffer
	int buffer = 0;
	for (SoundMixerSound &sound : sounds)
	{
		if (!sound.playing || sound.done)
			continue;

		limiters[buffer].left = &sound.left_limiter;
		limiters[buffer].right = &sound.right_limiter;
		priorities[buffer] = sound.compression_priority;

		extract_stereo_f32(sound, conversion_buffers + (buffer * mix_samples), len);

		++buffer;
	}

	// calculate limiters so that the waves can be compressed the right amount
	calculate_stereo_limiters(buffer, len, limiters, priorities);

	float staging[mix_samples]; // mix_samples is >= len
	for (int i = 0; i < mix_samples; ++i)
		staging[i] = 0.0f;

	// MIX!!
	for (int i = 0; i < buffer; ++i)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const float source_left = (conversion_buffers + (i * mix_samples))[frame] * *limiters[i].left;
			const float source_right = (conversion_buffers + (i * mix_samples))[frame + 1] * *limiters[i].right;

			staging[frame] += source_left;
			staging[frame + 1] += source_right;
		}
	}

	/*
	for (int frame = 0; frame < len; frame += 2)
	{
		// logging
		const float tolerance = 0.00f;
		if (staging[frame] > 1.0f - tolerance)
			fprintf(stderr, "left clip high: %.8f\n", staging[frame]);
		else if (staging[frame] < -1.0f + tolerance)
			fprintf(stderr, "left clip low: %.8f\n", staging[frame]);

		if (staging[frame + 1] > 1.0f - tolerance)
			fprintf(stderr, "right clip high: %.8f\n", staging[frame + 1]);
		else if (staging[frame + 1] < -1.0f + tolerance)
			fprintf(stderr, "right clip low: %.8f\n", staging[frame + 1]);
	}
	*/

	// convert f32 to s16 and clip
	for (int i = 0; i < len; ++i)
	{
		staging[i] = std::clamp(staging[i], -1.0f, 1.0f);

		dest[i] = (std::int16_t)(staging[i] * (staging[i] < 0.0f ? 32768.0f : 32767.0f));
	}

	// slowly bring the limiters back up
	for (int i = 0; i < buffer; ++i)
	{
		const float increase = micros_since_last_call / 1'000'000.0f;

		*limiters[i].left += increase;
		*limiters[i].right += increase;

		if (*limiters[i].left > 1.0f)
			*limiters[i].left = 1.0f;

		if (*limiters[i].right > 1.0f)
			*limiters[i].right = 1.0f;
	}

	// debugging schtuff
	{
		auto end = std::chrono::high_resolution_clock::now();

		static float accum = 0;
		static int cycles = 0;

		constexpr int report_rate = 4000;
		constexpr int budget_micros = 4000;

		accum += std::chrono::duration<float, std::ratio<1, 1000000>>(end - start).count();
		++cycles;

		if (cycles == report_rate)
		{
			const double micros = accum / (double)report_rate;
			fprintf(stderr, "Took %.2f micros (%.2f%% of budget)\n", micros, (micros / budget_micros) * 100);

			accum = 0;
			cycles = 0;
		}
	}

	return len;
}

void SoundMixer::calculate_stereo_limiters(const int count, const int len, const std::array<StereoLimiter, max_sounds> &limiters, const std::array<float, max_sounds> &priorities)
{
	float staging[mix_samples];
	for (int i = 0; i < mix_samples; ++i)
		staging[i] = 0.0f;

	// premix
	for (int i = 0; i < count; ++i)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			staging[frame] += (conversion_buffers + (i * mix_samples))[frame] * *limiters[i].left;
			staging[frame + 1] += (conversion_buffers + (i * mix_samples))[frame + 1] * *limiters[i].right;
		}
	}

	// determine overages
	float left_global_max = 0.0f, right_global_max = 0.0f;
	int left_global_max_position = 0, right_global_max_position = 0;

	for (int frame = 0; frame < len; frame += 2)
	{
		const float abs_left = std::abs(staging[frame]);
		const float abs_right = std::abs(staging[frame + 1]);

		if (abs_left > left_global_max)
		{
			left_global_max = abs_left;
			left_global_max_position = frame;
		}

		if (abs_right > right_global_max)
		{
			right_global_max = abs_right;
			right_global_max_position = frame + 1;
		}
	}

	const float left_global_overage = left_global_max - 1.0f;
	const float right_global_overage = right_global_max - 1.0f;

	if (left_global_overage <= 0.0f && right_global_overage <= 0.0f)
		return;

	// find the contributors
	bool left_overage_contributors[max_sounds];
	bool right_overage_contributors[max_sounds];
	for (int i = 0; i < count; ++i)
	{
		const float left_sample = (conversion_buffers + (i * mix_samples))[left_global_max_position] * *limiters[i].left;
		const float right_sample = (conversion_buffers + (i * mix_samples))[right_global_max_position] * *limiters[i].right;

		left_overage_contributors[i] = std::abs(left_sample) > 0.00001f;
		right_overage_contributors[i] = std::abs(right_sample) > 0.00001f;
	}

	// figure out the priorities / accountabilities
	float left_accountability_sum = 0.0f;
	float right_accountability_sum = 0.0f;
	for (int i = 0; i < count; ++i)
	{
		const float accountability = 1.0f - priorities[i];

		if (left_overage_contributors[i])
			left_accountability_sum += accountability;
		if (right_overage_contributors[i])
			right_accountability_sum += accountability;
	}

	const float left_accountability_multiplier = 1.0f / left_accountability_sum;
	const float right_accountability_multiplier = 1.0f / right_accountability_sum;
	float left_accountabilities[max_sounds];
	float right_accountabilities[max_sounds];
	for (int i = 0; i < count; ++i)
	{
		const float accountability = 1.0f - priorities[i];

		if (left_overage_contributors[i])
			left_accountabilities[i] = accountability * left_accountability_multiplier;
		if (right_overage_contributors[i])
			right_accountabilities[i] = accountability * right_accountability_multiplier;
	}

	bool more_compression_needed = false;
	// calculate limiters
	for (int i = 0; i < count; ++i)
	{
		if (left_global_overage > 0.0f && left_overage_contributors[i])
		{
			const float local_max = std::abs((conversion_buffers + (i * mix_samples))[left_global_max_position]) *
									*limiters[i].left;
			const float accountability = left_accountabilities[i] *
										 left_global_overage; // this wave is accountable for *this* much of the overage
			float target = local_max - accountability;
			if (target < 0.0f)
			{
				target = 0.0f;
				more_compression_needed = true;
			}

			const float limiter = target / local_max;
			*limiters[i].left = *limiters[i].left * limiter;
		}

		if (right_global_overage > 0.0f && right_overage_contributors[i])
		{
			const float local_max = std::abs((conversion_buffers + (i * mix_samples))[right_global_max_position]) *
									*limiters[i].right;
			const float accountability = right_accountabilities[i] *
										 right_global_overage; // this wave is accountable for *this* much of the overage
			float target = local_max - accountability;
			if (target < 0.0f)
			{
				target = 0.0f;
				more_compression_needed = true;
			}

			const float limiter = target / local_max;
			*limiters[i].right = *limiters[i].right * limiter;
		}
	}

	if (more_compression_needed)
		calculate_stereo_limiters(count, len, limiters, priorities); // go go tail call optimization
}

void SoundMixer::extract_stereo_f32(SoundMixerSound &sound, float *const dest, const int len)
{
	float extractbuf[mix_samples]; // mix_samples will be >= buf_len
	zero_float(extractbuf, mix_samples);

	const int channels = sound.sound.source.channels();
	const int read_samples = channels == 1 ? len / 2 : len;

	if (channels != 1 && channels != 2)
		win::bug("SoundMixer: mono or stereo PCM required");

	// extract the PCM data from the stream
	const int got = sound.sound.source.read_samples(extractbuf, read_samples);
	if (got < read_samples) // we were shorted a little bit
	{
		if (sound.sound.source.empty()) // the decoder is done and the stream is empty
		{
			if (sound.looping)
			{
				// restart the stream, for the next loop
				sound.sound.source.restart();

				// now that it's been restarted, see if we can squeeze a bit more out of it
				const int got2 = sound.sound.source.read_samples(extractbuf + got, read_samples - got);

				if (got + got2 < read_samples) // gol dangit we were shorted again
					zero_float(extractbuf + got + got2, read_samples - (got + got2)); // blank out the rest, this will cause a small skip in the audio. oh well
			}
			else
			{
				zero_float(extractbuf + got, read_samples - got); // blank out the rest
				sound.done = true; // mark this sound as completed.
			}
		}
		else
			zero_float(extractbuf + got, read_samples - got); // blank out the rest, this will cause a small skip in the audio. oh well
	}

	// apply volume config
	if (channels == 1)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const float sample = extractbuf[frame / 2];

			dest[frame] = sample * sound.left;
			dest[frame + 1] = sample * sound.right;
		}
	}
	else
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const float left = extractbuf[frame];
			const float right = extractbuf[frame + 1];

			dest[frame] = left * sound.left;
			dest[frame + 1] = right * sound.right;
		}
	}
}

void SoundMixer::zero_float(float *f, int len)
{
	for (int i = 0; i < len; ++i)
		f[i] = 0.0f;
}

}
