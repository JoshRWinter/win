#include <math.h>

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#include "win.h"

static void decodeogg(std::unique_ptr<unsigned char[]>, unsigned long long, short*, unsigned long long, std::atomic<unsigned long long>*);

static void default_sound_config_fn(float, float, float, float, float *volume, float *balance)
{
	*volume = 1.0f;
	*balance = 0.0f;
}

static float clamp_volume(float v)
{
	if(v > 1.0f)
		return 1.0f;
	else if(v < 0.0f)
		return 0.0f;

	return v;
}

static float clamp_balance(float bal)
{
	if(bal > 1.0f)
		return 1.0f;
	else if(bal < -1.0f)
		return -1.0f;

	return bal;
}

win::audio_engine::audio_engine(audio_engine &&rhs)
{
	move_common(rhs);
	move_platform(rhs);
}

win::audio_engine::~audio_engine()
{
	for(apack &ap : imported_)
		for(int i = 0; i < ap.count; ++i)
			ap.stored[i].thread.join();

	finalize();
}

win::audio_engine &win::audio_engine::operator=(audio_engine &&rhs)
{
	finalize();

	move_common(rhs);
	move_platform(rhs);

	return *this;
}

void win::audio_engine::import(const data_list &list)
{
	imported_.push_back({});
	apack &apack = imported_[imported_.size() - 1];
	apack.count = list.count();
	apack.stored = std::make_unique<stored_sound[]>(list.count());

	for(int i = 0; i < list.count(); ++i)
	{
		data raw = list.get(i);
		const unsigned long long file_size = raw.size();

		apack.stored[i].size = 0;
		apack.stored[i].encoded = std::make_unique<unsigned char[]>(raw.size());
		if(raw.read(apack.stored[i].encoded.get(), file_size) != file_size)
			bug("Could not read entire ogg file");
		raw.finalize(); // early destruct (optional)

		// eventual size of decoded data
		unsigned long long index = file_size - 1;
		while(!(apack.stored[i].encoded[index] == 'S' && apack.stored[i].encoded[index - 1] == 'g' && apack.stored[i].encoded[index - 2] == 'g' && apack.stored[i].encoded[index - 3] == 'O'))
			--index;
		unsigned long long samplecount;
		memcpy(&samplecount, &apack.stored[i].encoded[index + 3], sizeof(samplecount));
		apack.stored[i].target_size = samplecount * sizeof(short);

		apack.stored[i].buffer = std::make_unique<short[]>(samplecount);

		// decode
		std::thread thread(decodeogg, std::move(apack.stored[i].encoded), file_size, apack.stored[i].buffer.get(), apack.stored[i].target_size, &apack.stored[i].size);
		apack.stored[i].thread = std::move(thread);
	}
}

void win::audio_engine::get_config(float listenerx, float listenery, float sourcex, float sourcey, float *volume_l, float *volume_r)
{
	float volume = 0.0f, balance = 0.0f;

	config_fn_(listenerx, listenery, sourcex, sourcey, &volume, &balance);

	// clamp [0.0, 1.0f]
	volume = clamp_volume(volume);

	// clamp [-1.0f, 1.0f]
	balance = clamp_balance(balance);

	// convert to volumes
	*volume_l = volume;
	*volume_r = volume;

	if(balance > 0.0f)
		*volume_l -= balance;
	else if(balance < 0.0f)
		*volume_r += balance;

	// reclamp
	*volume_l = clamp_volume(*volume_l);
	*volume_r = clamp_volume(*volume_r);
}

// move the member data that is common to all platforms
void win::audio_engine::move_common(audio_engine &rhs)
{
	next_id_ = rhs.next_id_;
	listener_x_ = rhs.listener_x_;
	listener_y_ = rhs.listener_y_;
	config_fn_ = rhs.config_fn_;
	imported_ = std::move(rhs.imported_);
}

/* ------------------------------------*/
/////////////////////////////////////////
///// LINUX /////////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

#if defined WINPLAT_LINUX

static void raise(const std::string &msg)
{
	throw win::exception("PulseAudio: " + msg);
}

static void callback_connect(pa_context*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
}

static void callback_stream(pa_stream*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
}

static void callback_stream_drained(pa_stream*, int success, void *data)
{
	win::sound *snd = (win::sound*)data;
	snd->drained.store(success == 1);
}

static size_t channel_dupe(void *d, const void *s, size_t len)
{
	if(len % 4 != 0)
		win::bug("len not divisible by 4: " + std::to_string(len));

	char *const dest = (char*)d;
	const char *const source = (const char*)s;

	for(size_t i = 0; i < len; i += 4)
	{
		dest[i + 0] = source[(i / 2) + 0];
		dest[i + 1] = source[(i / 2) + 1];
		dest[i + 2] = source[(i / 2) + 0];
		dest[i + 3] = source[(i / 2) + 1];
	}

	return len;
}

static void callback_stream_write(pa_stream *stream, const size_t bytes, void *data)
{
	win::sound *const snd = (win::sound*)data;

	if(snd->start == snd->target_size)
		return;

	size_t total_written = 0;
	while(total_written != bytes)
	{
		// see if there even is any more data in the source buffer
		if(snd->start == snd->target_size)
			break;

		// spinlock until more data is ready
		while(snd->size->load() - snd->start == 0);

		const size_t left_to_write = bytes - total_written; // how many more bytes i owe pulseaudio
		const size_t total_size = snd->size->load(); // total size of the source buffer
		const size_t i_have = total_size - snd->start; // i have this many bytes from the source buffer ready to write
		size_t take = left_to_write / 2; // i want to give pulseaudio this much data taken from the source buffer
		if(take > i_have)
			take = i_have; // i must readjust how much i want to take from source buffer
		size_t give = take * 2; // how much data pulseaudio will get from me
		char *dest = NULL;

		if(pa_stream_begin_write(snd->stream, (void**)&dest, &give) || dest == NULL)
			win::bug("pa_stream_begin_write() failure");

		take = give / 2; // pulseaudio has changed its mind about how much it wants
		channel_dupe(dest, ((char*)snd->pcm) + snd->start, give);

		if(pa_stream_write(snd->stream, dest, give, NULL, 0, PA_SEEK_RELATIVE))
			win::bug("pa_stream_write() failure");

		// update
		snd->start += take;
		total_written += give;
	}

	// see if the stream is done
	if(snd->start == snd->target_size)
	{
		pa_operation *op = pa_stream_drain(stream, callback_stream_drained, data);
		if(!op)
			raise("Couldn't drain the stream");
		pa_operation_unref(op);
	}
}

win::audio_engine::audio_engine()
{
	context_ = NULL;
}

win::audio_engine::audio_engine(sound_config_fn fn)
{
	next_id_ = 1;
	listener_x_ = 0.0f;
	listener_y_ = 0.0f;
	if(fn == NULL)
		config_fn_ = default_sound_config_fn;
	else
		config_fn_ = fn;

	// loop
	loop_ = pa_threaded_mainloop_new();
	if(loop_ == NULL)
		raise("Could not initialize process loop");
	pa_mainloop_api *api = pa_threaded_mainloop_get_api(loop_);

	// pa context
	context_ = pa_context_new(api, "pcm-playback");
	if(context_ == NULL)
		raise("Could not create PA context");

	// start the loop
	pa_context_set_state_callback(context_, callback_connect, loop_);
	pa_threaded_mainloop_lock(loop_);
	if(pa_threaded_mainloop_start(loop_))
		raise("Could not start the process loop");

	if(pa_context_connect(context_, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL))
		raise("Could not connect the PA context");

	// wait for the context
	for(;;)
	{
		const pa_context_state_t context_state = pa_context_get_state(context_);
		if(context_state == PA_CONTEXT_READY)
			break;
		else if(context_state == PA_CONTEXT_FAILED)
			raise("Context connection failed");
		pa_threaded_mainloop_wait(loop_);
	}

	pa_threaded_mainloop_unlock(loop_);
}

// ambient (for music)
int win::audio_engine::play(sound_key id, bool looping)
{
	return play(id, true, looping, 0.0f, 0.0f);
}

// stero (for in-world sounds)
int win::audio_engine::play(sound_key id, float x, float y, bool looping)
{
	return play(id, false, looping, x, y);
}

int win::audio_engine::play(sound_key id, bool ambient, bool looping, float x, float y)
{
	if(id.apackno >= (int)imported_.size() || id.apackno < 0)
		bug("Apack id out of bounds");
	if(id.id >= imported_[id.apackno].count || id.id < 0)
		bug("Sound id out of bounds");

	if(sounds_.size() > MAX_SOUNDS)
	{
		pa_threaded_mainloop_lock(loop_);
		cleanup(false);
		pa_threaded_mainloop_unlock(loop_);
	}

	if(sounds_.size() > MAX_SOUNDS)
		return -1;

	const int sid = next_id_++;
	char namestr[16];
	snprintf(namestr, sizeof(namestr), "%d", sid);

	pa_sample_spec spec;
	spec.format = PA_SAMPLE_S16LE;
	spec.channels = 2;
	spec.rate = 44100;

	pa_buffer_attr attr;
	attr.maxlength = (std::uint32_t) -1;
	attr.tlength = (std::uint32_t) -1;
	attr.prebuf = (std::uint32_t) -1;
	attr.minreq = (std::uint32_t) -1;

	const unsigned flags = PA_STREAM_START_CORKED;

	pa_threaded_mainloop_lock(loop_);

	// cleanup dead streams
	cleanup(false);

	pa_stream *stream = pa_stream_new(context_, namestr, &spec, NULL);
	if(stream == NULL)
		raise("Could not create stream object");

	sound &stored = sounds_.emplace_front(this, sid, looping, 0, imported_[id.apackno].stored[id.id].buffer.get(), &imported_[id.apackno].stored[id.id].size, imported_[id.apackno].stored[id.id].target_size, stream, ambient, x, y);

	pa_stream_set_state_callback(stream, callback_stream, loop_);
	pa_stream_set_write_callback(stream, callback_stream_write, &stored);

	if(pa_stream_connect_playback(stream, NULL, &attr, (pa_stream_flags)flags, NULL, NULL))
		raise("Could not connect the playback stream");

	for(;;)
	{
		pa_stream_state_t state = pa_stream_get_state(stream);
		if(state == PA_STREAM_READY)
			break;
		else if(state == PA_STREAM_FAILED)
			raise("Stream connection failed");
		pa_threaded_mainloop_wait(loop_);
	}

	pa_cvolume volume;
	pa_cvolume_init(&volume);
	pa_cvolume_set(&volume, 2, PA_VOLUME_NORM);
	pa_operation_unref(pa_context_set_sink_input_volume(context_, pa_stream_get_index(stream), &volume, NULL, NULL));

	pa_operation_unref(pa_stream_cork(stream, 0, [](pa_stream*, int, void*){}, loop_));
	while(pa_stream_is_corked(stream));
	pa_threaded_mainloop_unlock(loop_);

	return sid;
}

void win::audio_engine::pause()
{
	auto callback = [](pa_stream*, int, void*) {};

	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
	{
		pa_operation *op = pa_stream_cork(snd.stream, 1, callback, NULL);
		if(!op)
			raise("Couldn't pause the stream");

		while(pa_operation_get_state(op) != PA_OPERATION_DONE);
		pa_operation_unref(op);
	}
	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::resume()
{
	auto callback = [](pa_stream*, int, void*) {};

	for(sound &snd : sounds_)
	{
		pa_operation *op = pa_stream_cork(snd.stream, 0, callback, NULL);
		if(!op)
			raise("Couldn't unpause the stream");

		while(pa_operation_get_state(op) != PA_OPERATION_DONE);
		pa_operation_unref(op);
	}
}

void win::audio_engine::pause(int id)
{
	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
	{
		if(id != snd.id)
			continue;

		if(!pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

		break;
	}
	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::resume(int id)
{
	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
	{
		if(id != snd.id)
			continue;

		if(pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 0, NULL, NULL));

		continue;
	}
	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::source(int id, float x, float y)
{
	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
	{
		if(snd.id != id)
			continue;

		snd.x = x;
		snd.y = y;

		float volume_left;
		float volume_right;
		get_config(listener_x_, listener_y_, x, y, &volume_left, &volume_right);

		pa_cvolume volume;
		volume.channels = 2;
		volume.values[0] = PA_VOLUME_NORM * volume_left;
		volume.values[1] = PA_VOLUME_NORM * volume_right;

		pa_operation_unref(pa_context_set_sink_input_volume(context_, pa_stream_get_index(snd.stream), &volume, NULL, NULL));
		break;
	}
	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::listener(float x, float y)
{
	listener_x_ = x;
	listener_y_ = y;

	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
	{
		float volume_left;
		float volume_right;
		get_config(x, y, snd.x, snd.y, &volume_left, &volume_right);

		pa_cvolume volume;
		volume.channels = 2;
		volume.values[0] = PA_VOLUME_NORM * volume_left;
		volume.values[1] = PA_VOLUME_NORM * volume_right;

		pa_operation_unref(pa_context_set_sink_input_volume(context_, pa_stream_get_index(snd.stream), &volume, NULL, NULL));
	}
	pa_threaded_mainloop_unlock(loop_);
}

// move the platform specific (pulseaudio) data members
void win::audio_engine::move_platform(audio_engine &rhs)
{
	sounds_ = std::move(rhs.sounds_);

	context_ = rhs.context_;
	loop_ = rhs.loop_;

	rhs.context_ = NULL;
	rhs.loop_ = NULL;
}

void win::audio_engine::cleanup(bool all)
{
	for(auto it = sounds_.begin(); it != sounds_.end();)
	{
		sound &snd = *it;

		const bool done = snd.drained; // the sound is done playing

		if(!all) // only cleaning up sounds that have finished
			if(!done)
			{
				++it;
				continue; // skip it if it's not done playing
			}

		if(!done)
		{
			// flush pending audio data
			pa_operation *op_flush = pa_stream_flush(snd.stream, [](pa_stream*, int, void*){}, NULL);
			if(!op_flush)
				raise("Couldn't flush the stream");

			// tell pulse audio it's done
			pa_operation *op_drain = pa_stream_drain(snd.stream, callback_stream_drained, (void*)&snd);
			if(!op_drain)
				raise("Couldn't drain the stream");

			pa_threaded_mainloop_unlock(loop_);

			// wait for flush
			while(pa_operation_get_state(op_flush) != PA_OPERATION_DONE);
			pa_operation_unref(op_flush);

			// wait for drain
			while(!snd.drained);
			pa_operation_unref(op_drain);

			pa_threaded_mainloop_lock(loop_);
		}

		pa_threaded_mainloop_unlock(loop_);
		while(!snd.drained);
		pa_threaded_mainloop_lock(loop_);

		if(pa_stream_disconnect(snd.stream))
			raise("Couldn't disconnect stream");
		pa_stream_unref(snd.stream);

		it = sounds_.erase(it);
	}
}

void win::audio_engine::finalize()
{
	if(context_ == NULL)
		return;

	pa_threaded_mainloop_lock(loop_);
	cleanup(true);
	pa_threaded_mainloop_unlock(loop_);

	pa_threaded_mainloop_stop(loop_);
	pa_context_disconnect(context_);
	pa_threaded_mainloop_free(loop_);
	pa_context_unref(context_);

	context_ = NULL;
}

#elif defined WINPLAT_WINDOWS

#endif

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
		raise("Input does not appear to be an Ogg bitstream");
	}

	ogg_stream_init(&stream, ogg_page_serialno(&page));

	vorbis_info_init(&info);
	vorbis_comment_init(&comment);
	if(ogg_stream_pagein(&stream, &page) < 0)
		raise("Could not read the first page of the Ogg bitstream data");

	if(ogg_stream_packetout(&stream, &packet) != 1)
		raise("Could not read initial header packet");

	if(vorbis_synthesis_headerin(&info, &comment, &packet) < 0)
		raise("This Ogg bitstream does not contain Vorbis audio data");

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
						raise("Corrupt secondary header");

					result = vorbis_synthesis_headerin(&info, &comment, &packet);
					if(result < 0)
						raise("Corrupt secondary header");

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
			raise("EOF before reading all Vorbis headers");

		ogg_sync_wrote(&state, bytes);
	}

	if(info.channels != 1)
		raise("Only mono-channels audio is supported");

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
					raise("Corrupt or missing data in the bitstream");
				else
				{
					ogg_stream_pagein(&stream, &page);

					while(1)
					{
						result = ogg_stream_packetout(&stream, &packet);

						if(result == 0)
							break;
						if(result < 0)
							raise("Corrupt or missing data in the bitstream");
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
		raise("Corrupt header during playback initialization");

	ogg_stream_clear(&stream);
	vorbis_comment_clear(&comment);
	vorbis_info_clear(&info);
	ogg_sync_clear(&state);
}