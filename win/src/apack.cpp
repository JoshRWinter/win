#include <math.h>

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#include <win.h>

static void decodeogg(std::unique_ptr<unsigned char[]>, unsigned long long, short*, unsigned long long, std::atomic<unsigned long long>*);

win::apack::apack()
	: count_(0)
{
}

win::apack::apack(const data_list &list)
{
	count_ = list.count();
	stored_ = std::make_unique<apack_sound[]>(5);

	for(int i = 0; i < list.count(); ++i)
	{
		apack_sound &stored = stored_[i];

		data raw = list.get(i);
		const unsigned long long file_size = raw.size();

		stored.size = 0;
		stored.encoded.reset(raw.release());

		// eventual size of decoded data
		unsigned long long index = file_size - 1;
		while(!(stored.encoded[index] == 'S' && stored.encoded[index - 1] == 'g' && stored.encoded[index - 2] == 'g' && stored.encoded[index - 3] == 'O'))
			--index;
		unsigned long long samplecount;
		memcpy(&samplecount, &stored.encoded[index + 3], sizeof(samplecount));
		stored.target_size = samplecount * sizeof(short);

		stored.buffer = std::make_unique<short[]>(samplecount);

		// decode
		stored.thread = std::thread(decodeogg, std::move(stored.encoded), file_size, stored.buffer.get(), stored.target_size, &stored.size);
	}
}

win::apack::apack(apack &&rhs)
{
	move(rhs);
}

win::apack::~apack()
{
	finalize();
}

win::apack &win::apack::operator=(apack &&rhs)
{
	finalize();
	move(rhs);
	return *this;
}

void win::apack::move(apack &rhs)
{
	if(rhs.count_ > 0)
	// join the decoding threads because they reference this class's memory
	for(int i = 0; i < rhs.count_; ++i)
		if(rhs.stored_[i].thread.joinable())
			rhs.stored_[i].thread.join();

	count_ = rhs.count_;
	stored_ = std::move(rhs.stored_);

	rhs.count_ = 0;
}

void win::apack::finalize()
{
	for(int i = 0; i < count_; ++i)
		if(stored_[i].thread.joinable())
			stored_[i].thread.join();

	count_ = 0;
}

static void ogg_vorbis_error(const std::string &msg)
{
	throw win::exception("Ogg-Vorbis: " + msg);
}

void decodeogg(std::unique_ptr<unsigned char[]> encoded_ptr, const unsigned long long encoded_size, short *const decoded, const unsigned long long decoded_size, std::atomic<unsigned long long> *const size){
	const unsigned char *const encoded = encoded_ptr.get();

	ogg_sync_state state; // oy
	ogg_stream_state stream; // os
	ogg_page page; // og
	ogg_packet packet; // op
	vorbis_info info; // vi
	vorbis_comment comment; // vc
	vorbis_dsp_state dsp; // vd
	vorbis_block block; // vb

	char *buffer;
	int bytes;

	ogg_sync_init(&state);

	int eos = 0;
	int i;

	buffer = ogg_sync_buffer(&state, 4096);
	bytes = std::min(encoded_size, (long long unsigned)4096);
	memcpy(buffer, encoded, bytes);
	unsigned long long index = bytes;

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
		if(encoded_size - index >= 4096)
			bytes = 4096;
		else
			bytes = encoded_size - index;
		memcpy(buffer, encoded + index, bytes);
		index += bytes;

		if(bytes < 4096 && i < 2)
			ogg_vorbis_error("EOF before reading all Vorbis headers");

		ogg_sync_wrote(&state, bytes);
	}

	if(info.channels != 1)
		ogg_vorbis_error("Only mono-channels audio is supported");

	const long long convsize = 4096 / info.channels;
	std::unique_ptr<std::int16_t[]> convbuffer(new std::int16_t[4096]);
	long long offset = 0; // offset into <decoded>

	if(vorbis_synthesis_init(&dsp, &info) == 0)
	{
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
				else
				{
					ogg_stream_pagein(&stream, &page);

					while(1)
					{
						result = ogg_stream_packetout(&stream, &packet);

						if(result == 0)
							break;
						if(result < 0)
							ogg_vorbis_error("Corrupt or missing data in the bitstream");
						else
						{
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
									ogg_int16_t *ptr = convbuffer.get() + i;

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

								if(offset + (bout * 2 * info.channels) > (long long)decoded_size)
									std::cerr << ("write overflow: size = " + std::to_string(decoded_size) + ", offset =  " + std::to_string(offset) + ", " + std::to_string(bout * 2 * info.channels) + " bytes") << std::endl;
								else
									memcpy((char*)(decoded) + offset, convbuffer.get(), bout * 2 * info.channels);
								offset += bout * 2 * info.channels;
								size->store(offset);

								vorbis_synthesis_read(&dsp, bout);
							}
						}
					}

					if(ogg_page_eos(&page))
						eos = 1;
				}
			}

			if(!eos)
			{
				buffer = ogg_sync_buffer(&state, 4096);
				if(encoded_size - index >= 4096)
					bytes = 4096;
				else
					bytes = encoded_size - index;

				memcpy(buffer, encoded + index, bytes);
				index += bytes;
				ogg_sync_wrote(&state, bytes);
				if(bytes == 0)
					eos = 1;
			}
		}

		vorbis_block_clear(&block);
		vorbis_dsp_clear(&dsp);
	}
	else
		ogg_vorbis_error("Corrupt header during playback initialization");

	ogg_stream_clear(&stream);
	vorbis_comment_clear(&comment);
	vorbis_info_clear(&info);
	ogg_sync_clear(&state);
}
