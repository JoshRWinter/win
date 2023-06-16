#include <algorithm>
#include <cmath>

#include <vorbis/vorbisfile.h>

#include <win/Win.hpp>
#include <win/sound/DecodingPcmSource.hpp>

namespace win
{

DecodingPcmSource::DecodingPcmSource(Stream data, int seek_start, int cached_channels, bool compute_total_size)
	: cancel(false)
	, reset(false)
	, finished(false)
	, channel_count(cached_channels)
	, channels_initialized_signal(1)
	, total_size(compute_total_size ? compute_size(data) : -1)
{
	worker = std::move(std::thread(decodeogg_loop, std::ref(*this), std::move(data), seek_start));
}

DecodingPcmSource::~DecodingPcmSource()
{
	stop();
}

long DecodingPcmSource::pcm_size() const
{
	if (total_size == -1)
		win::bug("DecodingPcmSource: No size");

	return total_size;
}

void DecodingPcmSource::stop()
{
	cancel.store(true);
	if (worker.joinable())
		worker.join();
}

int DecodingPcmSource::channels()
{
	const int c = channel_count.load();

	if (c != -1)
		return c;

	channels_initialized_signal.wait();

	const int c2 = channel_count.load();
	if (c2 == -1)
		win::bug("DecodingPcmSource: bad channels initialization");

	return c2;
}

void DecodingPcmSource::restart()
{
	if (!finished.load())
		win::bug("DecodingPcmSource does not support restarting if it is not empty!");

	finished.store(false);
	reset.store(true);
}

bool DecodingPcmSource::empty()
{
	return finished.load() && buffer.size() == 0;
}

int DecodingPcmSource::read_samples(float *buf, int samples)
{
	return buffer.read(buf, samples);
}

long DecodingPcmSource::compute_size(Stream &stream)
{
	const int spot = stream.tell();

	for (int index = stream.size() - 4; index >= 0; --index)
	{
		stream.seek(index);

		unsigned char bytes[4];
		stream.read(bytes, 4);

		if (bytes[0] == 'O' && bytes[1] == 'g' && bytes[2] == 'g' && bytes[3] == 'S')
		{
			stream.seek(index + 6);

			if (index + 6 + 8 > stream.size())
				win::bug("Couldn't determine size");

			std::uint64_t samplecount;
			stream.read(&samplecount, sizeof(samplecount));

			if (samplecount > std::numeric_limits<std::int64_t>::max())
				win::bug("Too big");

			stream.seek(spot);

			return (long)samplecount;
		}
	}

	win::bug("Couldn't determine size");
}

void DecodingPcmSource::set_channels(int c)
{
	if (channel_count.load() != -1)
		return; // don't bother double-setting

	channel_count.store(c);
	channels_initialized_signal.count_down();
}

[[noreturn]] static void ogg_vorbis_error(const std::string &msg)
{
	win::bug("Ogg-Vorbis: " + msg);
}

void DecodingPcmSource::decodeogg_loop(win::DecodingPcmSource &parent, win::Stream datafile, int seek_to)
{
	decodeogg(parent, datafile, seek_to);
	parent.finished.store(true);

	while (!parent.cancel.load())
	{
		bool expected = true;
		if (parent.reset.compare_exchange_strong(expected, false))
		{
			datafile.seek(0);
			decodeogg(parent, datafile, seek_to);
			parent.finished.store(true);
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void DecodingPcmSource::decodeogg(win::DecodingPcmSource &parent, win::Stream &datafile, int skip_samples)
{
	ogg_sync_state state;
	ogg_stream_state stream;
	ogg_page page;
	ogg_packet packet;
	vorbis_info info;
	vorbis_comment comment;
	vorbis_dsp_state dsp;
	vorbis_block block;

	char *buffer;
	int bytes;

	ogg_sync_init(&state);

	int eos = 0;
	int i;

	buffer = ogg_sync_buffer(&state, 4096);
	bytes = std::min(datafile.size(), (long long unsigned)4096);
	datafile.read(buffer, bytes);

	ogg_sync_wrote(&state, bytes);

	if(ogg_sync_pageout(&state, &page) != 1)
	{
		ogg_vorbis_error("Input does not appear to be an Ogg bitstream");
	}

	ogg_stream_init(&stream, ogg_page_serialno(&page));

	vorbis_info_init(&info);
	vorbis_comment_init(&comment);
	if(ogg_stream_pagein(&stream, &page) < 0)
		ogg_vorbis_error("Could not read the first page of the Ogg bitstream data");

	if(ogg_stream_packetout(&stream, &packet) != 1)
		ogg_vorbis_error("Could not read initial header packet");

	if(vorbis_synthesis_headerin(&info, &comment, &packet) < 0)
		ogg_vorbis_error("This Ogg bitstream does not contain Vorbis audio data");

	i = 0;
	while(i < 2)
	{
		while(i < 2)
		{
			int result = ogg_sync_pageout(&state, &page);
			if(result == 0)
				break;
			if(result == 1)
			{
				ogg_stream_pagein(&stream, &page);

				while(i < 2)
				{
					result = ogg_stream_packetout(&stream, &packet);
					if(result == 0)
						break;
					if(result < 0)
						ogg_vorbis_error("Corrupt secondary header");

					result = vorbis_synthesis_headerin(&info, &comment, &packet);
					if(result < 0)
						ogg_vorbis_error("Corrupt secondary header");

					++i;
				}
			}
		}

		buffer = ogg_sync_buffer(&state, 4096);
		if(datafile.size() - datafile.tell() >= 4096)
			bytes = 4096;
		else
			bytes = datafile.size() - datafile.tell();
		datafile.read(buffer, bytes);

		if(bytes < 4096 && i < 2)
			ogg_vorbis_error("EOF before reading all Vorbis headers");

		ogg_sync_wrote(&state, bytes);
	}

	if (info.channels != 1 && info.channels != 2)
		ogg_vorbis_error("Only mono or stereo data");

	// set the parent's channel_count
	parent.set_channels(info.channels);

	const long long convsize = 4096 / info.channels;
	std::int16_t convbuffer[4096];

	if(vorbis_synthesis_init(&dsp, &info) != 0)
		ogg_vorbis_error("Corrupt header during playback initialization");

	vorbis_block_init(&dsp, &block);
	while(!eos)
	{
		while(!eos)
		{
			int result = ogg_sync_pageout(&state, &page);
			if(result == 0)
				break;
			if(result < 0)
				ogg_vorbis_error("Corrupt or missing data in the bitstream");

			ogg_stream_pagein(&stream, &page);

			while(1)
			{
				result = ogg_stream_packetout(&stream, &packet);

				if(result == 0)
					break;
				if(result < 0)
					ogg_vorbis_error("Corrupt or missing data in the bitstream");

				float **pcm;
				int samples;

				if(vorbis_synthesis(&block, &packet) == 0)
					vorbis_synthesis_blockin(&dsp, &block);

				while((samples = vorbis_synthesis_pcmout(&dsp, &pcm)) > 0)
				{
					int j;
					int bout = samples < convsize ? samples : convsize;

					for(i = 0; i < info.channels; ++i)
					{
						ogg_int16_t *ptr = convbuffer + i;

						float *mono = pcm[i];
						for(j = 0; j < bout; ++j)
						{
							int val = std::floor(mono[j] * 32767.0f + 0.5f);
							if(val > 32767)
								val = 32767;
							else if(val < -32768)
								val = -32768;

							*ptr = val;
							ptr += info.channels;
						}
					}

					unsigned long long samples_written = 0;
					const unsigned long long want_to_write_samples = bout * info.channels;

					// remember to eat the first <skip_samples> samples
					if (skip_samples > 0)
					{
						const int eat = std::min((unsigned long long)skip_samples, want_to_write_samples);
						samples_written += eat;
						skip_samples -= eat;
					}

					float floatconv[4096];
					static_assert(sizeof(floatconv) == sizeof(convbuffer) * 2, "mismatch");
					for (int k = 0; k < want_to_write_samples; ++k)
						floatconv[k] = convbuffer[k] / (convbuffer[k] < 0 ? 32768.0f : 32767.0f);

					for(;;)
					{
						samples_written += parent.buffer.write(floatconv + samples_written, want_to_write_samples - samples_written);

						if (samples_written == want_to_write_samples)
							break;

						if (parent.cancel.load())
							goto cleanup;

						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}

					vorbis_synthesis_read(&dsp, bout);
				}
			}

			if(ogg_page_eos(&page))
				eos = 1;
		}

		if(!eos)
		{
			buffer = ogg_sync_buffer(&state, 4096);
			unsigned long long bigness = datafile.size();
			unsigned long long place = datafile.tell();
			if(bigness - place >= 4096)
				bytes = 4096;
			else
				bytes = bigness - place;

			datafile.read(buffer, bytes);
			ogg_sync_wrote(&state, bytes);
			if(bytes == 0)
				eos = 1;
		}
	}

cleanup:
	vorbis_block_clear(&block);
	vorbis_dsp_clear(&dsp);

	ogg_stream_clear(&stream);
	vorbis_comment_clear(&comment);
	vorbis_info_clear(&info);
	ogg_sync_clear(&state);
}

}
