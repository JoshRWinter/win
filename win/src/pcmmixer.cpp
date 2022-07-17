#include <limits>

#include <win/pcmmixer.hpp>

namespace win
{

PCMMixer::PCMMixer(win::AssetRoll &roll)
	: cache(roll)
{
	static_assert(mix_samples % 2 == 0, "conversion_buffer_len must be divisible by 2");
	conversion_buffers_owner.reset(new std::int16_t[max_streams * mix_samples]);
	conversion_buffers = conversion_buffers_owner.get();
}

int PCMMixer::add(win::SoundPriority priority, const char *name, float left, float right, bool looping)
{
	cleanup(false);

	PCMStream &stream = cache.load_stream(name);
	const auto key = streams.add(priority, stream, std::min(1.0f, left), std::min(1.0f, right), looping);

	return key;
}

void PCMMixer::config(std::uint32_t key, float left, float right)
{
}

void PCMMixer::pause(std::uint32_t key)
{
}

void PCMMixer::resume(std::uint32_t key)
{
}

void PCMMixer::cleanup(bool all)
{
	for (auto stream = streams.begin(); stream != streams.end();)
	{
		const bool kill = stream->done || all;

		if (kill)
		{
			cache.unload_stream(stream->stream);
			stream = streams.remove(stream);
			continue;
		}

		++stream;
	}
}

void PCMMixer::remove(std::uint32_t key)
{
	PCMMixerStream *stream = streams[key];
	if (stream == NULL)
		return;

	stream->done = true;
}

// called by audio callback thread
// all other methods are called by "main" thread
int PCMMixer::mix_stereo(std::int16_t *const dest, int len)
{
	len = std::min(len, mix_samples); // cap it
	if (len % 2 != 0) --len; // make sure it's even
	if (len <= 0) return 0;

	// zero out the input
	memset(dest, 0, len * sizeof(std::int16_t));

	// to store the left/right volumes for the streams
	struct vol { float left, right; };
	vol volumes[max_streams];

	int buffer = 0;
    for (PCMMixerStream &stream : streams)
	{
		if (!stream.playing)
			continue;

		if (stream.done)
			continue;

		volumes[buffer].left = stream.left;
		volumes[buffer].right = stream.right;

		if (stream.stream.channels() == 2)
		{
			win::bug("STEREO");
			const int got = stream.stream.read_samples((conversion_buffers + (buffer * mix_samples)), len);
			if (got < len) // just blank out what we couldn't get
				memset((conversion_buffers + (buffer * mix_samples)) + got, 0, (len - got) * sizeof(std::int16_t));
		}
		else // assume mono
		{
			const int half = len / 2;
			std::int16_t convbuf[mix_samples / 2];

			const int got = stream.stream.read_samples(convbuf, half);
			if (got < half)
			{
				// check if the stream is done
				if (stream.stream.is_writing_completed() && stream.stream.size() == 0)
				{
					fprintf(stderr, "reached end of stream\n");

					if (stream.looping)
					{
						fprintf(stderr, "loop\n");
						stream.stream.reset();

						const int got2 = stream.stream.read_samples(convbuf + got, half - got);
						if (got + got2 < half)
							memset(convbuf + got + got2, 0, (half - (got + got2)) * sizeof(std::int16_t));
					}
					else
						stream.done = true;
				}
				else // otherwise just blank out what we couldn't get
					memset(convbuf + got, 0, (half - got) * sizeof(std::int16_t));
			}

			for (int frame = 0; frame < len; frame += 2)
			{
				(conversion_buffers + (buffer * mix_samples))[frame + 0] = convbuf[(frame / 2)];
				(conversion_buffers + (buffer * mix_samples))[frame + 1] = convbuf[(frame / 2)];
			}
		}

		++buffer;
	}

	// sir mix-a-lot
	for (int i = 0; i < buffer; ++i)
	{
		for (int frame = 0; frame < len; frame += 2)
		{
			const int left_source = (conversion_buffers + (i * mix_samples))[frame] * volumes[i].left;
			const int right_source = (conversion_buffers + (i * mix_samples))[frame + 1] * volumes[i].right;

			const int left_dest = dest[frame];
			const int right_dest = dest[frame + 1];

			const std::int16_t left_clipped = std::min((int)std::numeric_limits<std::int16_t>::max(), left_dest + left_source);
			const std::int16_t right_clipped = std::min((int)std::numeric_limits<std::int16_t>::max(), right_dest + right_source);

			if (left_clipped != right_clipped)
				win::bug("NOT MONO!");

			/*
			if (left_clipped == std::numeric_limits<std::int16_t>::max())
				fprintf(stderr, "left clip\n");
			if (right_clipped == std::numeric_limits<std::int16_t>::max())
				fprintf(stderr, "right clip\n");
				*/

			dest[frame] = left_clipped;
			dest[frame + 1] = right_clipped;
		}
	}

    return len;
}

}
