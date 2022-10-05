#include <limits>
#include <cmath>

#include <random>
#include <win/soundmixer.hpp>

namespace win
{

SoundMixer::SoundMixer(win::AssetRoll &roll)
	: cache(roll)
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

int SoundMixer::add(const char *name, win::SoundResidencyPriority residency_priority, float compression_priority, float left, float right, bool looping, int seek)
{
	cleanup(false);

	Sound &sound = cache.load(name, seek);
	const auto key = sounds.add(sound, residency_priority, compression_priority, std::max(left, 0.0f), std::max(right, 0.0f), looping);
	if (key == -1)
	{
		// nevermind, need to unload it
		cache.unload(sound);
	}

	return key;
}

void SoundMixer::config(std::uint32_t key, float left, float right)
{
}

void SoundMixer::pause(std::uint32_t key)
{
}

void SoundMixer::resume(std::uint32_t key)
{
}

void SoundMixer::cleanup(bool all)
{
	for (auto sound = sounds.begin(); sound != sounds.end();)
	{
		const bool kill = sound->done || all;

		if (kill)
		{
			cache.unload(sound->sound);
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
/*
int SoundMixer::mix_stereo(std::int16_t *const dest, int len)
{
	const auto now = std::chrono::high_resolution_clock::now();
	auto start = now;

	len = std::min(len, mix_samples); // cap it
	if (len % 2 != 0) --len; // make sure it's even
	if (len <= 0) return 0;

	const float micros_since_last_call = std::chrono::duration<float, std::ratio<1, 1'000'000>>(now - last_call).count();
	last_call = now;

	// to store the left/right volumes for the streams
	struct vol { float left, right; };
	vol volumes[max_sounds];
	float accountabilities[max_sounds];

	/*
	 * Stage 1.
	 * Pull <mix_samples> of raw audio data out of the streams.
	 * If there isn't enough data in the stream, pad the end with zeros. Causes "skips" in the sound but oh well. Makes the code simpler
	 * Double-up mono data and make it stereo (duplicate the channels)
	 * Handle looping and marking streams as "done"

	int buffer = 0;
	for (SoundMixerSound &sound : sounds)
	{
		if (!sound.playing)
			continue;

		if (sound.done)
			continue;

		volumes[buffer].left = sound.left;
		volumes[buffer].right = sound.right;

		accountabilities[buffer] = 1.0f / sound.compression_priority;

		const int stream_channels = sound.sound.stream.channels();
		if (stream_channels == 2)
		{
			const int got = sound.sound.stream.read_samples((conversion_buffers + (buffer * mix_samples)), len);

			if (got < len)
			{
				if (sound.sound.stream.is_writing_completed() && sound.sound.stream.size() == 0)
				{
					fprintf(stderr, "SoundMixer: reached end of stream\n");

					if (sound.looping)
					{
						fprintf(stderr, "SoundMixer: loop\n");
						sound.sound.source.reset();

						const int got2 = sound.sound.stream.read_samples(conversion_buffers + (buffer * mix_samples) + got, len - got);
						if (got + got2 < len)
							memset(conversion_buffers + (buffer * mix_samples) + got + got2, 0, (len - (got + got2)) * sizeof(std::int16_t)); // allow a skip
					}
					else
					{
						memset(conversion_buffers + (buffer * mix_samples) + got, 0, (len - got) * sizeof(std::int16_t));
						sound.done = true;
					}
				}
				else
					memset((conversion_buffers + (buffer * mix_samples)) + got, 0, (len - got) * sizeof(std::int16_t)); // allow a skip
			}
		}
		else if (stream_channels == 1)
		{
			const int half = len / 2;
			std::int16_t convbuf[mix_samples / 2];

			const int got = sound.sound.stream.read_samples(convbuf, half);
			if (got < half)
			{
				// check if the stream is done
				if (sound.sound.stream.is_writing_completed() && sound.sound.stream.size() == 0)
				{
					fprintf(stderr, "SoundMixer: reached end of stream\n");

					if (sound.looping)
					{
						fprintf(stderr, "SoundMixer: loop\n");
						sound.sound.source.reset();

						const int got2 = sound.sound.stream.read_samples(convbuf + got, half - got);
						if (got + got2 < half)
							memset(convbuf + got + got2, 0, (half - (got + got2)) * sizeof(std::int16_t)); // allow a skip
					}
					else
					{
						memset(convbuf + got, 0, (half - got) * sizeof(std::int16_t));
						sound.done = true;
					}
				}
				else
					memset(convbuf + got, 0, (half - got) * sizeof(std::int16_t)); // allow a skip
			}

			for (int frame = 0; frame < len; frame += 2)
			{
				(conversion_buffers + (buffer * mix_samples))[frame + 0] = convbuf[(frame / 2)];
				(conversion_buffers + (buffer * mix_samples))[frame + 1] = convbuf[(frame / 2)];
			}
		}
		else
			win::bug("SoundMixer: invalid channels");

		++buffer;
	}

	/*
	 * Stage 2.
	 * Amplitude compression
	int global_overage;

	do
	{
		/*
		 * Stage 2a.
		 * Determine global overage
		int staging[mix_samples];
		memset(staging, 0, sizeof(staging));

		int left_global_max = 0;
		int right_global_max = 0;
		int left_global_max_position;
		int right_global_max_position;

		for (int i = 0; i < sounds.size(); i++)
		{
			for (int frame = 0; frame < len; frame += 2)
			{
				// do left
				const int left = (conversion_buffers + (i * mix_samples))[frame] * volumes[i].left; // possible overflow
				staging[frame] += left;

				if (std::abs(staging[frame]) > left_global_max)
				{
					left_global_max = std::abs(staging[frame]);
					left_global_max_position = frame;
				}

				// do right
				const int right = (conversion_buffers + (i * mix_samples))[frame + 1] * volumes[i].right; // possible overflow
				staging[frame + 1] += right;

				if (std::abs(staging[frame + 1]) > right_global_max)
				{
					right_global_max = std::abs(staging[frame + 1]);
					right_global_max_position = frame + 1;
				}
			}
		}

		/*
		 * Stage 2b.
		 * Compute limiters to account for the overage


		// notes for next time.
		// convert to float. will help with rounding errors
		// continue with naiveish compression

		left_global_overage = global_max - std::numeric_limits<std::int16_t>::max();
		int unaccounted = 0;

		if (global_overage > 0)
		{
			//fprintf(stderr, "overage of %d\n", global_overage);

			float priority_sum = 0;
			for (int i = 0; i < sounds.size(); ++i)
				priority_sum += accountabilities[i];

			const float multiplier = 1.0f / priority_sum;

			for (int i = 0; i < sounds.size(); ++i)
			{
				const float volume_adjust = global_max_position % 2 == 0 ? volumes[i].left : volumes[i].right;
				const int local_max = std::abs((conversion_buffers + (i * mix_samples))[global_max_position] * volume_adjust); // highest point in this wave (for the section being processed by this function call)
				const float adjusted_accountability = accountabilities[i] * multiplier; // this wave is *this* percent accountable for the global overage
				const int shed = std::ceil(adjusted_accountability * global_overage); // get rid of *this* much of the amplitude of the wave
				const int target = std::max(0, local_max - shed); // compress the wave so that the highest point is *this*

				const float limiter = target / (float)local_max;
				limiters[i] = std::min(limiters[i], limiter);
		}
	} while (global_overage > 0);

	/*
	 * Stage 3.
	 * Mix the waves together (in s32), and apply the limiters

	int staging2[mix_samples];
	memset(staging2, 0, sizeof(staging2));
	for (int i = 0; i < sounds.size(); ++i)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const int left_source = (conversion_buffers + (i * mix_samples))[frame] * volumes[i].left * limiters[i];
			const int right_source = (conversion_buffers + (i * mix_samples))[frame + 1] * volumes[i].right * limiters[i];

			const int left_dest = staging2[frame];
			const int right_dest = staging2[frame + 1];

			const int left = left_dest + left_source;
			const int right = right_dest + right_source;

			staging2[frame] = left;
			staging2[frame + 1] = right;
		}
	}

	/*
	 * Stage 4.
	 * Convert to s16

	for (int i = 0; i < mix_samples; ++i)
	{
		const int tolerance = 0;
		if (staging2[i] > std::numeric_limits<std::int16_t>::max() - tolerance || staging2[i] < std::numeric_limits<std::int16_t>::min() + tolerance)
			fprintf(stderr, "%s clip %d.\n", i % 2 == 0 ? "left" : "right", staging2[i]);

		dest[i] = staging2[i];
	}

	const float limiter_step = 0.0025f * (micros_since_last_call / 4000);
	// bring the limiters back up
	for (int i = 0; i < max_sounds; ++i)
	{
		limiters[i] = std::min(limiters[i] + limiter_step, 1.0f);
	}

	// debugging schtuff
	{
		auto end = std::chrono::high_resolution_clock::now();

		static int cycles = 0;
		static long long accum = 0;
		constexpr int period = 500;
		constexpr int budget = 4000;
		std::chrono::duration<float, std::ratio<1, 1000000>> diff = end - start;
		accum += diff.count();

		if (++cycles > period)
		{
			double micros = accum / (double)period;
			fprintf(stderr, "Took %.2f micros (%.2f%% of budget) \n", micros, (micros / budget) * 100);
			fprintf(stderr, "limiter is %.4f\n", limiters[0]);
			accum = 0;
			cycles = 0;
		}
	}

	return len;
}
*/

int SoundMixer::mix_stereo(std::int16_t *dest, int len)
{
	const auto start = std::chrono::high_resolution_clock::now();

	len = std::min(len, mix_samples); // cap it
	if (len % 2 != 0) --len; // make sure it's even
	if (len <= 0) return 0;

	const float micros_since_last_call = std::chrono::duration<float, std::ratio<1, 1'000'000>>(start - last_call).count();
	last_call = start;

	// fill up the conv buffer
	int buffer = 0;
	for (SoundMixerSound &sound : sounds)
	{
		if (!sound.playing || sound.done)
			continue;

		extract_stereo_f32(sound, conversion_buffers + (buffer * mix_samples), len);

		++buffer;
	}

	// compress the individual waves to fit them in the digital range
	compress_stereo(len);

	float staging[mix_samples]; // mix_samples is >= len
	for (int i = 0; i < mix_samples; ++i)
		staging[i] = 0.0f;

	// MIX!!
	for (int i = 0; i < buffer; ++i)
	{
		for (int j = 0; j < len; j += 2)
		{
			const float source_left = (conversion_buffers + (i * mix_samples))[j];
			const float source_right = (conversion_buffers + (i * mix_samples))[j + 1];

			staging[j] += source_left;
			staging[j + 1] += source_right;
		}
	}

	// clip
	for (int frame = 0; frame < len; frame += 2)
	{
		if (staging[frame] > 1.0f)
			staging[frame] = 1.0f;
		else if (staging[frame] < -1.0f)
			staging[frame] = -1.0f;

		if (staging[frame + 1] > 1.0f)
			staging[frame + 1] = 1.0f;
		else if (staging[frame + 1] < -1.0f)
			staging[frame + 1] = -1.0f;
	}

	// convert f32 to s16
	for (int j = 0; j < len; ++j)
	{
		dest[j] = staging[j] * (staging[j] < 0.0f ? 32768.0f : 32767);
	}

	// debugging schtuff
	{
		auto end = std::chrono::high_resolution_clock::now();

		static int cycles = 0;
		static long long accum = 0;
		constexpr int period = 500;
		constexpr int budget = 4000;
		std::chrono::duration<float, std::ratio<1, 1000000>> diff = end - start;
		accum += diff.count();

		if (++cycles > period)
		{
			double micros = accum / (double)period;
			fprintf(stderr, "Took %.2f micros (%.2f%% of budget)\n", micros, (micros / budget) * 100);
			accum = 0;
			cycles = 0;
		}
	}
	return len;
}

void SoundMixer::compress_stereo(const int len)
{
	const int count = sounds.size();

	float staging[mix_samples];
	for (int i = 0; i < len; ++i)
		staging[i] = 0.0f;

	// premix
	for (int sound = 0; sound < count; ++sound)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			staging[frame] += (conversion_buffers + (sound * mix_samples))[frame];
			staging[frame + 1] += (conversion_buffers + (sound * mix_samples))[frame + 1];
		}
	}

	// determine overages
	float left_global_max = 0.0f, right_global_max = 0.0f;
	int left_global_max_position, right_global_max_position;

	for (int frame = 0; frame < len; frame += 2)
	{
		const float abs_left = std::abs(staging[frame]);
		const float abs_right = std::abs(staging[frame + 1]);

		if (abs_left > 1.0f)
		{
			left_global_max = abs_left;
			left_global_max_position = frame;
		}

		if (abs_right > 1.0f)
		{
			right_global_max = abs_right;
			right_global_max_position = frame + 1;
		}
	}

	if (left_global_max > 1.0f)
		fprintf(stderr, "left channel over by %.4f\n", left_global_max - 1.0f);
	if (right_global_max > 1.0f)
		fprintf(stderr, "right channel over by %.4f\n", right_global_max - 1.0f);

	const float left_global_overage = left_global_max - 1.0f;
	const float right_global_overage = right_global_max - 1.0f;

	if (left_global_overage > 0.0f || right_global_overage > 0.0f)
		return;

	// find the non-contributers
	bool left_non_contributers[max_sounds];
	bool right_non_contributers[max_sounds];
	for (int i = 0; i < count; ++i)
	{
		const float left_sample = (conversion_buffers + (i * mix_samples))[left_global_max_position];
		const float right_sample = (conversion_buffers + (i * mix_samples))[right_global_max_position];

		left_non_contributers[i] = std::abs(left_sample) < 0.0001f;
		right_non_contributers[i] = std::abs(right_sample) < 0.0001f;
	}


	todo: factor in the priority nonsense


	for (int i = 0; i < count; ++i)
	{
		if (left_global_overage > 0.0f)
		{
			const float local_max = std::abs((conversion_buffers + (i * mix_samples))[left_global_max_position]);
			const float accountability = accountabilities[i] * left_global_overage; // this channel is accountable for *this* much of the overage
			const float new_target = local_max - accountability > 0.0f ?: 0.0f;

			limiters[i].left = new_target / local_max;
		}
	}
}

void SoundMixer::extract_stereo_f32(SoundMixerSound &sound, float *const buf, const int buf_len)
{
	std::int16_t extractbuf[mix_samples]; // mix_samples will be >= buf_len
	memset(extractbuf, 0, sizeof(extractbuf));

	const int channels = sound.sound.stream.channels();
	const int read_samples = channels == 1 ? buf_len / 2 : buf_len;

	if (channels != 1 && channels != 2)
		win::bug("SoundMixer: mono or stereo PCM required");

	// extract the PCM data from the stream
	const int got = sound.sound.stream.read_samples(extractbuf, read_samples);
	if (got < read_samples) // we were shorted a little bit
	{
		if (sound.sound.stream.is_writing_completed() && sound.sound.stream.size() == 0) // the decoder is done and the stream is empty
		{
			if (sound.looping)
			{
				// restart the stream, for the next loop
				sound.sound.source.reset();

				// now that it's been restarted, see if we can squeeze a bit more out of it
				const int got2 = sound.sound.stream.read_samples(extractbuf + got, read_samples - got);

				if (got + got2 < read_samples) // gol dangit we were shorted again
					memset(extractbuf + got + got2, 0, (read_samples - (got + got2)) * sizeof(std::int16_t)); // blank out the rest, this will cause a small skip in the audio. oh well
			}
			else
			{
				memset(extractbuf + got, 0, (read_samples - got) * sizeof(std::int16_t)); // blank out the rest
				sound.done = true; // mark this sound as completed.
			}
		}
		else
			memset(extractbuf + got, 0, (read_samples - got) * sizeof(std::int16_t)); // blank out the rest, this will cause a small skip in the audio. oh well
	}

	// convert s16 to f32
	if (channels == 1)
	{
		for (int frame = 0; frame < buf_len; frame += 2)
		{
			const std::int16_t sample = extractbuf[frame / 2];

			buf[frame] = (sample / (sample < 0 ? 32768.0f : 32767.0f)) * sound.left;
			buf[frame + 1] = (sample / (sample < 0 ? 32768.0f : 32767.0f)) * sound.right;
		}
	}
	else
	{
		for (int frame = 0; frame < buf_len; frame += 2)
		{
			const std::int16_t left = extractbuf[frame];
			const std::int16_t right = extractbuf[frame + 1];

			buf[frame] = (left / (left < 0 ? 32768.0f : 32767.0f)) * sound.left;
			buf[frame + 1] = (right / (right < 0 ? 32768.0f : 32767.0f)) * sound.right;
		}
	}
}

}
