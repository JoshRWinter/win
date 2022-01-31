#include <algorithm>

#include <math.h>

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#include <win.h>

#ifdef WINPLAT_LINUX
#include <unistd.h>
#endif

namespace win
{

SoundPage::SoundPage(bool cache_this_page)
{
	this->cache_this_page = cache_this_page;
	read_only = false;
	samples_filled = 0;
	fake_samples_filled = 0;
	samples_consumed = 0;
	samples_owner.reset(new std::int16_t[PAGE_SAMPLE_COUNT]);
	samples = samples_owner.get();
}

SoundPage::SoundPage(std::int16_t *cached_samples, unsigned long long cached_samples_count)
{
	cache_this_page = false;
	read_only = true;
	samples_filled = cached_samples_count;
	fake_samples_filled = 0;
	samples_consumed = 0;
	samples = cached_samples;
}

Sound::Sound(Stream &&sourcefile, const std::string &soundname, SoundBank *parent)
	: parent(parent)
{
	name = soundname;
	channels = -1;
	writing_completed = false;

	// create the first page now
	pages.emplace_back(true);

	// decode
	decoder_worker_cancel.store(false);
	decoder_worker = std::thread(decodeogg, std::move(sourcefile), this, &decoder_worker_cancel);
}

Sound::Sound(const std::string &name, Stream *sourcefile, int channels, std::int16_t *samples, unsigned long long samples_count)
	: parent(NULL)
{
	const bool need_to_start_decoder = sourcefile != NULL;
	this->name = name;
	this->channels = channels;
	writing_completed = !need_to_start_decoder;

	// create the first page now
	pages.emplace_back(samples,  samples_count);

	// decode
	decoder_worker_cancel.store(false);
	if(need_to_start_decoder)
		decoder_worker = std::thread(decodeogg, std::move(*sourcefile), this, &decoder_worker_cancel);
}

Sound::~Sound()
{
	decoder_worker_cancel.store(true);
	if(decoder_worker.joinable())
		decoder_worker.join();

	if(parent != NULL && pages.size() > 0)
	{
		auto &frontpage = pages.front();
		if(frontpage.cache_this_page)
		{
			if(parent->cache_sound(name, channels, frontpage.samples, frontpage.samples_filled))
				frontpage.samples_owner.release(); // samples buffer is now owned by soundbank, need to release this
		}
	}
}

unsigned long long Sound::write(const std::int16_t *buffer, unsigned long long samples_to_write)
{
	SoundPage *write_page;
	int page_count;

	{
		std::lock_guard lock(pages_lock);

		page_count = pages.size();

		if (page_count == 0)
			pages.emplace_back();

		write_page = &pages.back();
	}

	if(write_page->read_only)
		return fake_write(buffer, samples_to_write, write_page);

	const unsigned long long samples_written = std::min(SoundPage::PAGE_SAMPLE_COUNT - write_page->samples_filled.load(), samples_to_write);

	memcpy(write_page->samples + write_page->samples_filled, buffer, samples_written * sizeof(std::int16_t));
	write_page->samples_filled += samples_written;

#ifndef NDEBUG
	if (write_page->samples_filled > SoundPage::PAGE_SAMPLE_COUNT)
		win::bug("Overwrote end of sound page");
#endif

	// make a new page if need be
	if(write_page->samples_filled == SoundPage::PAGE_SAMPLE_COUNT && page_count < MAX_PAGES)
	{
		{
			std::lock_guard lock(pages_lock);
			pages.emplace_back();
		}

		return samples_written + write(buffer + samples_written, samples_to_write - samples_written);
	}

	return samples_written;
}

unsigned long long Sound::fake_write(const std::int16_t *buffer, unsigned long long samples_to_write, SoundPage *write_page)
{
	const unsigned long long fake_samples_written = std::min(SoundPage::PAGE_SAMPLE_COUNT - write_page->fake_samples_filled.load(), samples_to_write);
	write_page->fake_samples_filled += fake_samples_written;

#ifndef NDEBUG
	if(write_page->fake_samples_filled > SoundPage::PAGE_SAMPLE_COUNT)
		win::bug("Fake overwrote end of sound page");
#endif

	if(write_page->fake_samples_filled == SoundPage::PAGE_SAMPLE_COUNT)
	{
		{
			std::lock_guard lock(pages_lock);
			pages.emplace_back();
		}

		return fake_samples_written + write(buffer + fake_samples_written, samples_to_write - fake_samples_written);
	}

	return fake_samples_written;
}

void Sound::complete_writing()
{
	writing_completed = true;
}

unsigned long long Sound::read(std::int16_t *buffer, unsigned long long request_samples)
{
	SoundPage *read_page;

	{
		std::lock_guard lock(pages_lock);

		if (pages.size() == 0)
			return 0;

		read_page = &pages.front();
	}

	const unsigned long long can_read_samples = std::min(read_page->samples_filled.load() - read_page->samples_consumed.load(), request_samples);
	memcpy(buffer, read_page->samples + read_page->samples_consumed, can_read_samples * sizeof(std::int16_t));
	read_page->samples_consumed += can_read_samples;

#ifndef NDEBUG
	if (read_page->samples_consumed > SoundPage::PAGE_SAMPLE_COUNT)
		win::bug("Overread end of sound page");
#endif

	if (read_page->samples_consumed == SoundPage::PAGE_SAMPLE_COUNT)
	{
		// this page is exhausted, delete it
		{
			std::lock_guard lock(pages_lock);

			// cache the page if it's the first one
			if(read_page->cache_this_page && parent != NULL)
			{
				if(parent->cache_sound(name, channels, read_page->samples, read_page->samples_filled))
					read_page->samples_owner.release(); // samples buffer is now owned by soundbank, need to release this
			}

			pages.pop_front();
		}

		if (can_read_samples < request_samples)
			return can_read_samples + read(buffer + can_read_samples, request_samples - can_read_samples);
	}

	return can_read_samples;
}

bool Sound::is_stream_completed()
{
	if(!writing_completed)
		return false;

	std::lock_guard lock(pages_lock);
	auto &lastpage = pages.back();
	return lastpage.samples_consumed.load() == lastpage.samples_filled.load();
}

}

[[noreturn]] static void ogg_vorbis_error(const std::string &msg)
{
	win::bug("Ogg-Vorbis: " + msg);
}

// this is copied from a vorbis decoder sample, god have mercy on ye
void decodeogg(win::Stream source, win::Sound *sound_p, std::atomic<bool> *cancel_p)
{
	std::atomic<bool> &cancel = *cancel_p;
	win::Sound &sound = *sound_p;

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

	sound.channels = info.channels;

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
						samples_written += sound.write(convbuffer + samples_written, want_to_write_samples - samples_written);

						if (samples_written == want_to_write_samples)
							break;

						if (cancel)
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
			if(source.size() - source.tell() >= 4096)
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

	sound.complete_writing();
}
