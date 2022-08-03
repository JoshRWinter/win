#include <cmath>

#include <vorbis/vorbisfile.h>

#include <win/win.hpp>
#include <win/pcmdecoder.hpp>
#include <win/ogg.hpp>

namespace win
{

PCMDecoder::PCMDecoder(PCMStream &target, PCMResource &resource, Stream *data, int seek_start)
	: target(target)
	, pcmresource(resource)
	, seek_start(seek_start)
    , cancel(false)
	, restart(false)
	, seek_to(seek_start)
{
	if (resource.is_completed())
	{
		// hydrate the stream
		fprintf(stderr, "PCMDecoder: rehydrating stream\n");

		const int fill = resource.fill();
		if (target.write_samples(resource.data(), fill) != fill)
			win::bug("PCMDecoder: Failed to rehydrate PCMStream");

		if (!resource.is_partial())
		{
			fprintf(stderr, "PCMDecoder: completely hydrated stream\n");

			target.complete_writing();
		}
	}

	// figure out whether to start the decoder
    if (!resource.is_completed() || resource.is_partial())
    {
		fprintf(stderr, "PCMDecoder: starting decoder\n");

		const int fill = resource.is_completed() ? resource.fill() : 0;
		const int channels = resource.channels();

		// potentially tell the decoder to skip what has already been cached
		seek_to.store(seek_start + fill);

	    if (channels == -1)
	    {
		    fprintf(stderr, "PCMDecoder: collecting channels\n");

		    impl::OneshotSignal channel_signal;
	    	worker = std::move(std::thread(decodeogg_loop, std::ref(*this), std::move(*data), &channel_signal));
	    	channel_signal.wait();
		}
		else
		{
			target.set_channels(channels);
	    	worker = std::move(std::thread(decodeogg_loop, std::ref(*this), std::move(*data), (impl::OneshotSignal*)NULL));
		}
	}
	else
	{
		// decoder will not be started. fill in the channels since no one else is going to.

		target.set_channels(resource.channels());
	}
}

PCMDecoder::~PCMDecoder()
{
	cancel.store(true);
	if (worker.joinable())
		worker.join();
}

void PCMDecoder::reset()
{
	// enforce some invariants
	if (!pcmresource.is_completed())
		win::bug("PCMDecoder: reset on incomplete pcm resource!");
	if (!target.is_writing_completed())
		win::bug("PCMDecoder: reset before pcm stream finished!");
	if (target.size() != 0)
		win::bug("PCMDecoder: reset on non-empty PCMStream!");

	target.reset();

	if (pcmresource.is_partial())
	{
		const int fill = pcmresource.fill();

		// rehydrate stream
		if (target.write_samples(pcmresource.data(), fill) != fill)
			win::bug("PCMDecoder: Failed to rehydrate PCMStream");

		seek_to.store(seek_start + fill);
		restart.store(true);
	}
	else // resource is "full" instead of partial
	{
		// no need to restart the ogg decoder
		// just rehydrate stream
		const int fill = pcmresource.fill();
		if (target.write_samples(pcmresource.data(), fill) != fill)
			win::bug("PCMDecoder: Failed to rehydrate PCMStream");

		target.complete_writing();
	}
}

PCMResource &PCMDecoder::resource()
{
	return pcmresource;
}

int PCMDecoder::write_samples(const std::int16_t *samples, int len)
{
	const int put = target.write_samples(samples, len);

	// also save this data to the "resource"
	// the resource caches pcm data
	pcmresource.write_samples(samples, put);

	return put;
}

void PCMDecoder::set_channels(int c)
{
	if (pcmresource.channels() != -1)
		win::bug("PCMDecoder: channels already set");

	pcmresource.set_channels(c);
	target.set_channels(c);
}

void PCMDecoder::complete_writing()
{
	if (!pcmresource.is_completed())
		pcmresource.complete();

	target.complete_writing();
}

#if defined WINPLAT_WINDOWS
#include <windows.h>
void platform_sleep(int millis) { Sleep(millis); }
#elif defined WINPLAT_LINUX
#include <unistd.h>
static void platform_sleep(int millis) { usleep(millis * 1000); }
#endif

void PCMDecoder::decodeogg_loop(win::PCMDecoder &parent, win::Stream datafile, impl::OneshotSignal *channel_signal)
{
    decodeogg(parent, datafile, parent.seek_to.load(), channel_signal);

	while (!parent.cancel.load())
	{
		if (parent.restart.load())
		{
			parent.restart.store(false);
			decodeogg(parent, datafile, parent.seek_to.load(), NULL);
		}
		else
			platform_sleep(100);
	}
}

[[noreturn]] static void ogg_vorbis_error(const std::string &msg)
{
	win::bug("Ogg-Vorbis: " + msg);
}

void PCMDecoder::decodeogg(win::PCMDecoder &parent, win::Stream &datafile, int skip_samples, impl::OneshotSignal *channel_signal)
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

	// if it's null, then the parent is not interested in the channels
	// (because it was already informed on a past run)
	if (channel_signal != NULL)
	{
		parent.set_channels(info.channels);
		channel_signal->notify();
	}

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

					// remember to eat the first <skip_samples> samples
		    		if (skip_samples > 0)
					{
						const int eat = std::min((unsigned long long)skip_samples, want_to_write_samples);
						samples_written += eat;
						skip_samples -= eat;
					}

					for(;;)
					{
						samples_written += parent.write_samples(convbuffer + samples_written, want_to_write_samples - samples_written);

						if (samples_written == want_to_write_samples)
							break;

						if (parent.cancel.load())
							goto cleanup;

						platform_sleep(8);
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

	parent.complete_writing();
}

}