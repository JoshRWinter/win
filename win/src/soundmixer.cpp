#include <limits>

#include <win/soundmixer.hpp>

namespace win
{

SoundMixer::SoundMixer(win::AssetRoll &roll)
	: cache(roll)
{
	static_assert(mix_samples % 2 == 0, "conversion_buffer_len must be divisible by 2");
	conversion_buffers_owner.reset(new std::int16_t[max_sounds * mix_samples]);
	conversion_buffers = conversion_buffers_owner.get();
}

SoundMixer::~SoundMixer()
{
	cleanup(true);
}

int SoundMixer::add(const char *name, win::SoundResidencyPriority residency_priority, float compression_priority, float left, float right, bool looping, int seek)
{
	cleanup(false);

	Sound &sound = cache.load(name, seek);
	const auto key = sounds.add(sound, residency_priority, compression_priority, std::max(std::min(left, 1.0f), 0.0f), std::max(std::min(right, 1.0f), 0.0f), looping);
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
int SoundMixer::mix_stereo(std::int16_t *const dest, int len)
{
	len = std::min(len, mix_samples); // cap it
	if (len % 2 != 0) --len; // make sure it's even
	if (len <= 0) return 0;

	// zero out the input
	memset(dest, 0, len * sizeof(std::int16_t));

	// to store the left/right volumes for the streams
	struct vol { float left, right; };
	vol volumes[max_sounds];

	int buffer = 0;
    for (SoundMixerSound &sound : sounds)
	{
		if (!sound.playing)
			continue;

		if (sound.done)
			continue;

		volumes[buffer].left = sound.left;
		volumes[buffer].right = sound.right;

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

	// sir mix a lot
	for (int i = 0; i < buffer; ++i)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const int left_source = (conversion_buffers + (i * mix_samples))[frame] * volumes[i].left;
			const int right_source = (conversion_buffers + (i * mix_samples))[frame + 1] * volumes[i].right;

			const int left_dest = dest[frame];
			const int right_dest = dest[frame + 1];

			const int left = left_dest + left_source;
			const int right = right_dest + right_source;

			const int left_clipped_high = std::min((int)std::numeric_limits<std::int16_t>::max(), left);
			const int right_clipped_high = std::min((int)std::numeric_limits<std::int16_t>::max(), right);

			const std::int16_t left_clipped_high_low = std::max((int)std::numeric_limits<std::int16_t>::min(), left_clipped_high);
			const std::int16_t right_clipped_high_low = std::max((int)std::numeric_limits<std::int16_t>::min(), right_clipped_high);

			/*
			if (left_clipped != right_clipped)
				win::bug("NOT MONO!");
				*/

			/*
			if (left_clipped == std::numeric_limits<std::int16_t>::max())
				fprintf(stderr, "left clip\n");
			if (right_clipped == std::numeric_limits<std::int16_t>::max())
				fprintf(stderr, "right clip\n");
				*/

			dest[frame] = left_clipped_high_low;
			dest[frame + 1] = right_clipped_high_low;
		}
	}

    return len;
}

}
