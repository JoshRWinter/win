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

void SoundEngine::get_config(float listenerx, float listenery, float sourcex, float sourcey, float *volume_l, float *volume_r)
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

static void channel_dupe(std::int16_t *const dest, const std::int16_t *const source, const size_t source_len)
{
	for(size_t i = 0; i < source_len * 2; i += 2)
	{
		dest[i] = source[i / 2];
		dest[i + 1] = source[i / 2];
	}
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

SoundEngine::SoundEngine(Display &p, AssetRoll &asset_roll, SoundConfigFn fn)
	: soundbank(asset_roll)
{
	conversion_buffer_sample_count = 0;
	next_id = 1;
	listener_x = 0.0f;
	listener_y = 0.0f;
	//last_process = std::chrono::high_resolution_clock::now();
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

SoundEngine::~SoundEngine()
{
	pa_threaded_mainloop_lock(loop);
	cleanup(true);
	pa_threaded_mainloop_unlock(loop);

	if(sounds.size() != 0)
		win::bug("could not sweep up all streams");

	pa_threaded_mainloop_stop(loop);
	pa_context_disconnect(context);
	pa_threaded_mainloop_free(loop);
	pa_context_unref(context);
}

void SoundEngine::write_to_stream(ActiveSound &sound, const size_t bytes)
{
	const auto requested_samples = bytes / sizeof(std::int16_t);

	if(sound.sound->is_stream_completed())
	{
		if(sound.looping)
		{
			std::string name = sound.sound->name;
			soundbank.unload(*sound.sound);
			sound.sound = &soundbank.load(name.c_str());
		}
		else
			return;
	}

	if((requested_samples / 2) * 3 > conversion_buffer_sample_count)
	{
		conversion_buffer = std::move(std::make_unique<std::int16_t[]>((requested_samples / 2) * 3));
		conversion_buffer_sample_count = (requested_samples / 2) * 3;
	}

	if(sound.sound->channels.load() == 1)
	{
		// supe it up to 2 channels
		const auto mono_requested_samples = requested_samples / 2;

		auto readbuffer = conversion_buffer.get();
		auto stereobuffer = conversion_buffer.get() + mono_requested_samples;
		const auto samples_actually_read = sound.sound->read(readbuffer, mono_requested_samples);

		channel_dupe(stereobuffer, readbuffer, mono_requested_samples);

		if(pa_stream_write(sound.stream, stereobuffer, samples_actually_read * sizeof(std::int16_t) * 2, NULL, 0, PA_SEEK_RELATIVE))
			win::bug("pa_stream_write() failure");
	}
	else if(sound.sound->channels.load() == 2)
	{
		const auto samples_actually_read = sound.sound->read(conversion_buffer.get(), requested_samples);

		if(pa_stream_write(sound.stream, conversion_buffer.get(), samples_actually_read * sizeof(std::int16_t), NULL, 0, PA_SEEK_RELATIVE))
			win::bug("pa_stream_write() failure");
	}
	else win::bug("PulseAudio: invalid channels");
}

void SoundEngine::process()
{
	/*
	const auto now = std::chrono::high_resolution_clock::now();
	if(std::chrono::duration<double, std::milli>(now - last_process).count() < 10)
		return;
	last_process = now;
	*/

	pa_threaded_mainloop_lock(loop);

	cleanup(false);

	for (auto &sound : sounds)
	{
		const auto requested_bytes = pa_stream_writable_size(sound.stream);
		if (requested_bytes == (size_t)-1)
			continue;
		if (requested_bytes == 0)
			continue;

		write_to_stream(sound, requested_bytes);
	}

	pa_threaded_mainloop_unlock(loop);
}

// ambient (for music)
int SoundEngine::play(const char *path, bool looping)
{
	return play(path, true, looping, 0.0f, 0.0f);
}

// stereo (for in-world sounds)
int SoundEngine::play(const char *path, float x, float y, bool looping)
{
	return play(path, false, looping, x, y);
}

int SoundEngine::play(const char *path, bool ambient, bool looping, float x, float y)
{
	pa_threaded_mainloop_lock(loop);

	cleanup(false);

	if(sounds.size() > MAX_SOUNDS)
	{
		pa_threaded_mainloop_unlock(loop);
		return -1;
	}

	auto &sound = soundbank.load(path);

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

	pa_stream *stream = pa_stream_new(context, namestr, &spec, NULL);
	if(stream == NULL)
		win::bug("Could not create stream object");

	sounds.emplace_back(sound, stream, sid, ambient, looping, x, y);

	// wait for channels property to get filled in by another thread
	while(sound.channels == -1) ;

	pa_stream_set_state_callback(stream, callback_stream, loop);

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

	pa_threaded_mainloop_unlock(loop);

	return sid;
}

void SoundEngine::pause(int id)
{
	pa_threaded_mainloop_lock(loop);

	for(auto &snd : sounds)
	{
		if(id != snd.id)
			continue;

		if(!pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

		break;
	}

	pa_threaded_mainloop_unlock(loop);
}

void SoundEngine::resume(int id)
{
	pa_threaded_mainloop_lock(loop);

	for(auto &snd : sounds)
	{
		if(id != snd.id)
			continue;

		if(pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 0, NULL, NULL));

		continue;
	}

	pa_threaded_mainloop_unlock(loop);
}

void SoundEngine::stop(int id)
{
	pa_threaded_mainloop_lock(loop);

	for(auto &snd : sounds)
	{
		if(id != snd.id)
			continue;

		if(!pa_stream_is_corked(snd.stream))
			pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

		snd.stop = true;

		continue;
	}

	pa_threaded_mainloop_unlock(loop);
}

void SoundEngine::source(int id, float x, float y)
{
	pa_threaded_mainloop_lock(loop);

	for(auto &snd : sounds)
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

void SoundEngine::listener(float x, float y)
{
	listener_x = x;
	listener_y = y;

	pa_threaded_mainloop_lock(loop);

	for(auto &snd : sounds)
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

void SoundEngine::cleanup(bool all)
{
	pa_threaded_mainloop_lock(loop);
	for(auto it = sounds.begin(); it != sounds.end();)
	{
		auto &sound = *it;

		if(!all) // only cleaning up sounds that have finished
		{
			// check if this sound has fully exhausted its source stream
			const bool completed = sound.sound->is_stream_completed() && !sound.looping;

			if (!completed && !sound.stop)
			{
				++it;
				continue;
			}

			if(!sound.stop)
			{
				// check if a drain operation has been issued yet
				if(sound.drain_op == NULL)
					sound.drain_op = pa_stream_drain(sound.stream, [](pa_stream*, int success, void *data){ ((ActiveSound*)data)->drained = success != 0; }, &sound);

				bool drained = false;
				if(sound.drain_op != NULL)
				{
					bool op_completed = pa_operation_get_state(sound.drain_op) == PA_OPERATION_DONE;
					drained = op_completed && sound.drained;

					if(op_completed)
					{
						pa_operation_unref(sound.drain_op);
						sound.drain_op = NULL;
					}
				}

				if(!drained)
				{
					++it;
					continue; // skip it if it's not done playing
				}
			}
		}

		pa_stream_set_state_callback(sound.stream, [](pa_stream*, void*){}, NULL);

		if(pa_stream_disconnect(sound.stream))
			win::bug("Couldn't disconnect stream");
		pa_stream_unref(sound.stream);

		soundbank.unload(*sound.sound);
		it = sounds.erase(it);
	}
	pa_threaded_mainloop_unlock(loop);
}

#elif defined WINPLAT_WINDOWS

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

static std::string describe_dserr(HRESULT result)
{
	switch(result)
	{
	case DS_NO_VIRTUALIZATION: return "No virtualization";
	//case DS_INCOMPLETE: return "No virtualization";
	case DSERR_ACCESSDENIED: return "Access denied";
	case DSERR_ALLOCATED: return "Allocated";
	case DSERR_ALREADYINITIALIZED: return "Already initialized";
	case DSERR_BADFORMAT: return "Bad format";
	case DSERR_BADSENDBUFFERGUID: return "Bad send buffer GUID";
	case DSERR_BUFFERLOST: return "Buffer lost";
	case DSERR_BUFFERTOOSMALL: return "Buffer too small";
	case DSERR_CONTROLUNAVAIL: return "Control unavailable";
	case DSERR_DS8_REQUIRED: return "DS8 required";
	case DSERR_FXUNAVAILABLE: return "FX unavailable";
	case DSERR_GENERIC: return "Generic";
	case DSERR_INVALIDCALL: return "Invalid call";
	case DSERR_INVALIDPARAM: return "Invalid parameter";
	case DSERR_NOAGGREGATION: return "No aggregation";
	case DSERR_NODRIVER: return "No driver";
	case DSERR_NOINTERFACE: return "No interface";
	case DSERR_OBJECTNOTFOUND: return "Object not found";
	case DSERR_OTHERAPPHASPRIO: return "Other app has priority";
	case DSERR_OUTOFMEMORY: return "Out of memory";
	case DSERR_PRIOLEVELNEEDED: return "Priority level needed";
	case DSERR_SENDLOOP: return "Send loop";
	case DSERR_UNINITIALIZED: return "Uninitialized";
	case DSERR_UNSUPPORTED: return "Unsupported";
	default: return "Undetermined";
	}
}

SoundEngine::SoundEngine(Display &parent, AssetRoll &roll, SoundConfigFn fn)
	: soundbank(roll)
{
	next_id = 1;
	listener_x = 0.0f;
	listener_y = 0.0f;
	config_fn = fn;
	convbuffer = std::move(std::make_unique<std::int16_t[]>(CONVBUFFER_SAMPLES));

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
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 4;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	if(primary->SetFormat(&format) != DS_OK)
		win::bug("DirectSound: Could not set the primary buffer format");
}

SoundEngine::~SoundEngine()
{
	cleanup(true);
	primary->Release();
	context->Release();
}

unsigned long long SoundEngine::write_empty(ActiveSound &sound, unsigned long long bytes_left)
{
	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(sound.stream->Lock(sound.write_cursor, bytes_left, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
		win::bug("DirectSound: couldn't lock stream buffer for empty write");

	memset(buffer1, 0, size1);
	memset(buffer2, 0, size2);

	if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
		win::bug("DirectSound: couldn't unlock stream buffer from empty write");

	sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;

	return (size1 + size2) / 2;
}

std::ifstream samplesfile("c:\\users\\josh\\desktop\\samples.bin", std::ifstream::binary);

void SoundEngine::write_to_stream(ActiveSound &sound)
{
	if(sound.stop)
		return;

	DWORD play_cursor = 0;
	if(sound.stream->GetCurrentPosition(&play_cursor, NULL) != DS_OK)
		win::bug("DirectSound: Couldn't determine play cursor position");

	const int bytes_left = sound.firstwrite ? MAX_WRITE_SAMPLES : sound.write_cursor <= play_cursor ? play_cursor - sound.write_cursor : ((SOUND_BUFFER_BYTES - sound.write_cursor) + play_cursor);
	const int want_to_write_samples = bytes_left / sizeof(std::int16_t);
	const int will_write_samples = std::min((unsigned long long)want_to_write_samples, MAX_WRITE_SAMPLES);

	if(bytes_left == 0)
		return;

	if(sound.sound->is_stream_completed() && !sound.looping)
	{
		sound.silent_samples_written += write_empty(sound, bytes_left);
		if(sound.silent_samples_written > SOUND_BUFFER_SAMPLES)
			sound.stop = true;
		return;
	}

#ifndef NDEBUG
	if(sound.write_cursor % 4 != 0)
		win::bug("DirectSound: write_cursor not divisible by 4");
	if(play_cursor % 4 != 0)
		win::bug("DirectSound: play_cursor not divisible by 4");
	if(bytes_left % 4 != 0)
		win::bug("DirectSound: bytes_left not divisible by 4");
	if(want_to_write_samples % 2 != 0)
		win::bug("DirectSound: want_to_write_samples not divisible by 2");
	if(will_write_samples % 2 != 0)
		win::bug("DirectSound: will_write_samples not divisible by 2");
#endif

	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(sound.sound->channels == 1)
	{
		const auto mono_write_samples = will_write_samples / 2;
		auto got = sound.sound->read(convbuffer.get(), mono_write_samples);

		if(sound.sound->is_stream_completed() && sound.looping)
		{
			const std::string name = sound.sound->name;
			soundbank.unload(*sound.sound);
			sound.sound = &soundbank.load(name.c_str());
		}

		if(got == 0)
			return;

		sound.firstwrite = false;

		channel_dupe(convbuffer.get() + got, convbuffer.get(), got);

		if(sound.stream->Lock(sound.write_cursor, got * sizeof(std::int16_t) * 2, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
			win::bug("DirectSound: couldn't lock stream buffer");

		if(size1 + size2 != got * sizeof(std::int16_t) * 2)
			win::bug("DirectSound: sample request mismatch");

		memcpy(buffer1, convbuffer.get() + got, size1);
		memcpy(buffer2, convbuffer.get() + got + (size1 / sizeof(std::int16_t)), size2);

		if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
			win::bug("DirectSound: couldn't unlock stream buffer");

		sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;
	}
	else if(sound.sound->channels == 2)
	{
		auto got = sound.sound->read(convbuffer.get(), will_write_samples);

		if(sound.sound->is_stream_completed() && sound.looping)
		{
			const std::string name = sound.sound->name;
			soundbank.unload(*sound.sound);
			sound.sound = &soundbank.load(name.c_str());
		}

		if(got == 0)
			return;

		sound.firstwrite = false;

		if(sound.stream->Lock(sound.write_cursor, got * sizeof(std::int16_t), &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
			win::bug("DirectSound: couldn't lock stream buffer");

		if(size1 + size2 != got * sizeof(std::int16_t))
			win::bug("DirectSound: sample request mismatch");

		memcpy(buffer1, convbuffer.get(), size1);
		memcpy(buffer2, convbuffer.get() + (size1 / sizeof(std::int16_t)), size2);

		if(sound.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
			win::bug("DirectSound: couldn't unlock stream buffer");

		sound.write_cursor = (sound.write_cursor + size1 + size2) % SOUND_BUFFER_BYTES;
	}
	else
	{
		win::bug("DirectSound: unsupported channels");
	}
}

void SoundEngine::process()
{
	cleanup(false);
	for(auto &sound : sounds)
	{
		write_to_stream(sound);
	}
}

int SoundEngine::play(const char *name, bool loop)
{
	return play(name, true, loop, 0.0f, 0.0f);
}

int SoundEngine::play(const char *name, float x, float y, bool loop)
{
	return play(name, false, loop, x, y);
}

int SoundEngine::play(const char *name, bool ambient, bool looping, float x, float y)
{
	cleanup(false);
	if(sounds.size() >= MAX_SOUNDS)
		return -1;

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 16;
	format.nChannels = 2;
	format.nBlockAlign = 4;
	format.nAvgBytesPerSec = 44100 * 4;
	format.cbSize = 0;

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GETCURRENTPOSITION2;
	buffer.dwBufferBytes = SOUND_BUFFER_BYTES;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = &format;
	buffer.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer *tmp;
	if(context->CreateSoundBuffer(&buffer, &tmp, NULL) != DS_OK)
		win::bug("DirectSound: Could not create temp buffer");

	IDirectSoundBuffer8 *stream;
	tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&stream);
	tmp->Release();

	auto &soundstream = soundbank.load(name);
	auto &sound = sounds.emplace_back(soundstream, stream, next_id++, ambient, looping, x, y);

	while(soundstream.channels.load() == -1);

	write_to_stream(sound);

	sound.stream->Play(0, 0, DSBPLAY_LOOPING);

	return sound.id;
}

void SoundEngine::pause(int)
{
}

void SoundEngine::resume(int)
{
}

void SoundEngine::stop(int)
{
}

void SoundEngine::source(int, float, float)
{
}

void SoundEngine::listener(float x, float y)
{
	listener_x = x;
	listener_y = y;
}

void SoundEngine::cleanup(bool all)
{
	for(auto sound = sounds.begin(); sound != sounds.end();)
	{
		if(all || sound->stop)
		{
			fprintf(stderr, "deleted sound %d\n", sound->id);
			soundbank.unload(*sound->sound);
			sound = sounds.erase(sound);
			continue;
		}

		++sound;
	}
}

#endif
}
