#include <win.h>

#ifdef WINPLAT_LINUX
#include <unistd.h>
#endif

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

namespace win
{

void AudioEngine::get_config(float listenerx, float listenery, float sourcex, float sourcey, float *volume_l, float *volume_r)
{
	float volume = 0.0f, balance = 0.0f;

	config_fn(listenerx, listenery, sourcex, sourcey, &volume, &balance);

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

/* ------------------------------------*/
/////////////////////////////////////////
///// LINUX /////////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

#if defined WINPLAT_LINUX

static void callback_connect(pa_context*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
}

static void callback_stream(pa_stream*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
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
	win::clip *const snd = (win::clip*)data;

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
		const size_t total_size = snd->size->load(); // total size of the (decoded) source buffer
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
		if(snd->looping)
		{
			snd->start = 0;

			if(total_written < bytes)
				callback_stream_write(stream, bytes - total_written, data); // recurse
		}
		else
			snd->finished.store(true);
	}
}

AudioEngine::AudioEngine(Display &p, SoundConfigFn fn)
{
	next_id = 1;
	listener_x = 0.0f;
	listener_y = 0.0f;
	if(fn == NULL)
		config_fn = default_sound_config_fn;
	else
		config_fn = fn;

	// loop
	loop = pa_threaded_mainloop_new();
	if(loop == NULL)
		win::bug("Could not initialize process loop");
	pa_mainloop_api *api = pa_threaded_mainloop_get_api(loop);

	// pa context
	context = pa_context_new(api, "pcm-playback");
	if(context == NULL)
		win::bug("Could not create PA context");

	// start the loop
	pa_context_set_state_callback(context, callback_connect, loop);
	pa_threaded_mainloop_lock(loop);
	if(pa_threaded_mainloop_start(loop))
		win::bug("Could not start the process loop");

	if(pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL))
		win::bug("Could not connect the PA context");

	// wait for the context
	for(;;)
	{
		const pa_context_state_t context_state = pa_context_get_state(context);
		if(context_state == PA_CONTEXT_READY)
			break;
		else if(context_state == PA_CONTEXT_FAILED)
			win::bug("Context connection failed");
		pa_threaded_mainloop_wait(loop);
	}

	pa_threaded_mainloop_unlock(loop);
}

AudioEngine::~AudioEngine()
{
	if (context == NULL)
		return;

	pa_threaded_mainloop_lock(loop);
	cleanup(true);
	pa_threaded_mainloop_unlock(loop);

	if(clips.size() != 0)
		win::bug("could not sweep up all streams");

	pa_threaded_mainloop_stop(loop);
	pa_context_disconnect(context);
	pa_threaded_mainloop_free(loop);
	pa_context_unref(context);
}

// ambient (for music)
int AudioEngine::play(Sound &sound, bool looping)
{
	return play(sound, true, looping, 0.0f, 0.0f);
}

// stero (for in-world sounds)
int AudioEngine::play(Sound &sound, float x, float y, bool looping)
{
	return play(sound, false, looping, x, y);
}

int AudioEngine::play(Sound &sound, bool ambient, bool looping, float x, float y)
{
	pa_threaded_mainloop_lock(loop);

	cleanup(false);

	if(clips.size() > MAX_SOUNDS)
	{
		pa_threaded_mainloop_unlock(loop);
		return -1;
	}

	const int sid = next_id++;
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

	// cleanup dead streams
	cleanup(false);

	pa_stream *stream = pa_stream_new(context, namestr, &spec, NULL);
	if(stream == NULL)
		win::bug("Could not create stream object");

	clip &stored = clips.emplace_front(sid, looping, 0, sound.buffer.get(), &sound.size, sound.target_size, stream, ambient, x, y);

	pa_stream_set_state_callback(stream, callback_stream, loop);
	pa_stream_set_write_callback(stream, callback_stream_write, &stored);

	if(pa_stream_connect_playback(stream, NULL, &attr, (pa_stream_flags)0, NULL, NULL))
		win::bug("Could not connect the playback stream");

	for(;;)
	{
		pa_stream_state_t state = pa_stream_get_state(stream);
		if(state == PA_STREAM_READY)
			break;
		else if(state == PA_STREAM_FAILED)
			win::bug("Stream connection failed");
		pa_threaded_mainloop_wait(loop);
	}

	pa_cvolume volume;
	pa_cvolume_init(&volume);
	pa_cvolume_set(&volume, 2, PA_VOLUME_NORM);
	pa_operation_unref(pa_context_set_sink_input_volume(context, pa_stream_get_index(stream), &volume, NULL, NULL));

	// register a drain notification
	stored.drain = pa_stream_drain(stream, NULL, NULL);
	if(stored.drain == NULL)
		win::bug("couldn't start a drain op");

	pa_threaded_mainloop_unlock(loop);

	return sid;
}

void AudioEngine::pause()
{
	pa_threaded_mainloop_lock(loop);
	for(clip &snd : clips)
		pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

	pa_threaded_mainloop_unlock(loop);
}

void AudioEngine::resume()
{
	pa_threaded_mainloop_lock(loop);

	for(clip &snd : clips)
		pa_operation_unref(pa_stream_cork(snd.stream, 0, NULL, NULL));

	pa_threaded_mainloop_unlock(loop);
}

void AudioEngine::pause(int id)
{
	pa_threaded_mainloop_lock(loop);

	for(clip &snd : clips)
	{
		if(id != snd.id)
			continue;

		if(!pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

		break;
	}

	pa_threaded_mainloop_unlock(loop);
}

void AudioEngine::resume(int id)
{
	pa_threaded_mainloop_lock(loop);

	for(clip &snd : clips)
	{
		if(id != snd.id)
			continue;

		if(pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 0, NULL, NULL));

		continue;
	}

	pa_threaded_mainloop_unlock(loop);
}

void AudioEngine::source(int id, float x, float y)
{
	pa_threaded_mainloop_lock(loop);

	for(clip &snd : clips)
	{
		if(snd.id != id)
			continue;

		snd.x = x;
		snd.y = y;

		float volume_left;
		float volume_right;
		get_config(listener_x, listener_y, x, y, &volume_left, &volume_right);

		pa_cvolume volume;
		volume.channels = 2;
		volume.values[0] = PA_VOLUME_NORM * volume_left;
		volume.values[1] = PA_VOLUME_NORM * volume_right;

		pa_operation_unref(pa_context_set_sink_input_volume(context, pa_stream_get_index(snd.stream), &volume, NULL, NULL));
		break;
	}

	pa_threaded_mainloop_unlock(loop);
}

void AudioEngine::listener(float x, float y)
{
	listener_x = x;
	listener_y = y;

	pa_threaded_mainloop_lock(loop);

	for(clip &snd : clips)
	{
		float volume_left;
		float volume_right;
		get_config(x, y, snd.x, snd.y, &volume_left, &volume_right);

		pa_cvolume volume;
		volume.channels = 2;
		volume.values[0] = PA_VOLUME_NORM * volume_left;
		volume.values[1] = PA_VOLUME_NORM * volume_right;

		pa_operation_unref(pa_context_set_sink_input_volume(context, pa_stream_get_index(snd.stream), &volume, NULL, NULL));
	}

	pa_threaded_mainloop_unlock(loop);
}

void win::AudioEngine::cleanup(bool all)
{
	for(auto it = clips.begin(); it != clips.end();)
	{
		clip &snd = *it;

		const bool done = pa_operation_get_state(snd.drain) == PA_OPERATION_DONE && snd.finished.load();

		if(!all) // only cleaning up sounds that have finished
			if(!done)
			{
				++it;
				continue; // skip it if it's not done playing
			}

		// prevent pulseaudio from being a bastard
		pa_stream_set_state_callback(snd.stream, [](pa_stream*, void*){}, NULL);
		pa_stream_set_write_callback(snd.stream, [](pa_stream*, size_t, void*){}, NULL);

		pa_operation *op_flush = pa_stream_flush(snd.stream, NULL, NULL);
		if(!op_flush)
			win::bug("Couldn't flush the stream");

		// wait for flush & drain
		while(pa_operation_get_state(op_flush) != PA_OPERATION_DONE && pa_operation_get_state(snd.drain) != PA_OPERATION_DONE)
		{
			pa_threaded_mainloop_unlock(loop);
			usleep(50);
			pa_threaded_mainloop_lock(loop);
		}

		pa_operation_unref(op_flush);
		pa_operation_unref(snd.drain);

		if(pa_stream_disconnect(snd.stream))
			win::bug("Couldn't disconnect stream");
		pa_stream_unref(snd.stream);

		it = clips.erase(it);
	}
}

#elif defined WINPLAT_WINDOWS

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

static constexpr unsigned long long SOUND_BUFFER_SIZE = 6 * 44100 * sizeof(short); // seconds * sample rate * sample size
static constexpr unsigned long long MAX_WRITE_SIZE = SOUND_BUFFER_SIZE / 2;

static void write_buffer(win::clip &snd, const DWORD offset, const int bytes)
{
	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(snd.stream->Lock(offset, bytes, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
		win::bug("DirectSound: Couldn't lock buffer for writing");

	if(size1 + size2 != bytes)
		win::bug("DirectSound: Requested write = " + std::to_string(bytes) + ", actual write = " + std::to_string(size1 + size2));

	memcpy(buffer1, (char*)snd.pcm + snd.start, size1);
	memcpy(buffer2, (char*)snd.pcm + snd.start + size1, size2);

	if(snd.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
		win::bug("DirectSound: Couldn't unlock buffer after writing");
}

static void write_directsound(win::clip &snd)
{
	if(snd.start == snd.target_size)
		return;

	const int size = snd.size->load();
	const int want_to_write = std::min(size - snd.start, MAX_WRITE_SIZE);
	const int offset = snd.write_cursor;

	if(offset >= SOUND_BUFFER_SIZE)
		win::bug("offset too big");

	write_buffer(snd, offset, want_to_write);

	snd.start += want_to_write;
	snd.write_cursor = (snd.write_cursor + want_to_write) % SOUND_BUFFER_SIZE;
}

AudioEngine::AudioEngine(Display &parent, SoundConfigFn fn)
{
	parent.directsound = this;
	last_poke = std::chrono::high_resolution_clock::now();
	next_id = 1;
	listener_x = 0.0f;
	listener_y = 0.0f;
	config_fn = fn;

	if(DirectSoundCreate8(NULL, &context, NULL) != DS_OK)
		win::bug("Could not initialize DirectSound");

	if(context->SetCooperativeLevel(parent.window, DSSCL_PRIORITY) != DS_OK)
		win::bug("DirectSound: Could not set cooperation level");

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	buffer.dwBufferBytes = 0;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = NULL;
	buffer.guid3DAlgorithm = GUID_NULL;

	if(context->CreateSoundBuffer(&buffer, &primary, NULL) != DS_OK)
		win::bug("DirectSound: Could not create the primary sound buffer");

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 2;
	format.nBlockAlign = 2;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	if(primary->SetFormat(&format) != DS_OK)
		win::bug("DirectSound: Could not set the primary buffer format");
}

AudioEngine::~AudioEngine()
{
	cleanup();
	primary->Release();
	context->Release();
}

int AudioEngine::play(Sound &snd, bool loop)
{
	return play(snd, true, loop, 0.0f, 0.0f);
}

int AudioEngine::play(Sound &snd, float x, float y, bool loop)
{
	return play(snd, false, loop, x, y);
}

int AudioEngine::play(Sound &src, bool ambient, bool looping, float x, float y)
{
	if(clips.size() >= MAX_SOUNDS)
		return -1;

	const unsigned long long size = src.size.load();

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 16;
	format.nChannels = 1;
	format.nBlockAlign = 2;
	format.nAvgBytesPerSec = 44100 * 2;
	format.cbSize = 0;

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2;
	buffer.dwBufferBytes = SOUND_BUFFER_SIZE;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = &format;
	buffer.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer *tmp;
	if(context->CreateSoundBuffer(&buffer, &tmp, NULL) != DS_OK)
		win::bug("DirectSound: Could not create temp buffer");

	IDirectSoundBuffer8 *stream;
	tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&stream);
	tmp->Release();

	clip &snd = clips.emplace_back(next_id++, looping, 0, src.buffer.get(), &src.size, src.target_size, ambient, x, y, stream);

	write_directsound(snd);

	snd.stream->Play(0, 0, DSBPLAY_LOOPING);

	return snd.id;
}

void AudioEngine::pause()
{
}

void AudioEngine::resume()
{
}

void AudioEngine::pause(int)
{
}

void AudioEngine::resume(int)
{
}

void AudioEngine::source(int, float, float)
{
}

void AudioEngine::listener(float x, float y)
{
	listener_x = x;
	listener_y = y;
}


void AudioEngine::poke()
{
	const auto now = std::chrono::high_resolution_clock::now();

	if(std::chrono::duration<double, std::milli>(now - last_poke).count() < 10)
		return;

	last_poke = now;

	for(auto snd = clips.begin(); snd != clips.end();)
	{
		DWORD play_cursor = 0;
		if(snd->stream->GetCurrentPosition(&play_cursor, NULL) != DS_OK)
			win::bug("DirectSound: Couldn't determine play cursor position");

		const int write_cursor = snd->write_cursor < play_cursor ? (snd->write_cursor + SOUND_BUFFER_SIZE) : snd->write_cursor;
		const int bytes_left = write_cursor - play_cursor;
		if(bytes_left < 44100 * 2)
			write_directsound(*snd);
		if(bytes_left > MAX_WRITE_SIZE && snd->start == snd->target_size)
		{
			snd->finalize();
			snd = clips.erase(snd);
			continue;
		}

		++snd;
	}

	const double dur = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - now).count();
	if(dur > 5)
		std::cerr << "took longer than 5 milliseconds" << std::endl;
}

void AudioEngine::cleanup()
{
	for(auto snd = clips.begin(); snd != clips.end();)
	{
		snd->finalize();
		snd = clips.erase(snd);
	}
}

#endif
}
