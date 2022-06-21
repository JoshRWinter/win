#include <cmath>

#include <win/win.hpp>
#if defined WINPLAT_LINUX
#include <unistd.h>
#elif defined WINPLAT_WINDOWS
#include <windows.h>
#endif

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#include <win/ogg.hpp>

[[noreturn]] static void ogg_vorbis_error(const std::string &msg)
{
	win::bug("Ogg-Vorbis: " + msg);
}

// this is copied from a vorbis decoder sample, god have mercy on ye
void decodeogg(win::Stream source, win::PCMStream &target, std::condition_variable &signal, std::mutex &mutex)
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
	bytes = std::min(source.size(), (long long unsigned)4096);
	source.read(buffer, bytes);

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
		if(source.size() - source.tell() >= 4096)
			bytes = 4096;
		else
			bytes = source.size() - source.tell();
		source.read(buffer, bytes);

		if(bytes < 4096 && i < 2)
			ogg_vorbis_error("EOF before reading all Vorbis headers");

		ogg_sync_wrote(&state, bytes);
	}

	{
		std::lock_guard<std::mutex> guard(mutex);
		target.channel_count = info.channels;
	}
	signal.notify_all();

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
							int val = floor(mono[j] * 32767.0f + 0.5f);
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
					for(;;)
					{
						samples_written += target.write_samples(convbuffer + samples_written, want_to_write_samples - samples_written);

						if (samples_written == want_to_write_samples)
							break;

						if (target.decoder_cancel.load())
							goto cleanup;

#if defined WINPLAT_LINUX
						usleep(5000);
#elif defined WINPLAT_WINDOWS
						Sleep(10);
#endif
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
			unsigned long long bigness = source.size();
			unsigned long long place = source.tell();
			if(bigness - place >= 4096)
				bytes = 4096;
			else
				bytes = source.size() - source.tell();

			source.read(buffer, bytes);
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

	target.complete_writing();
}
