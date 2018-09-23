#include <win.h>


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
#ifdef WINPLAT_WINDOWS
	parent_ = NULL;
#endif
	move_common(rhs);
	move_platform(rhs);
}

win::audio_engine::~audio_engine()
{
	finalize();
}

win::audio_engine &win::audio_engine::operator=(audio_engine &&rhs)
{
	finalize();

	move_common(rhs);
	move_platform(rhs);

	return *this;
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
int win::audio_engine::play(apack &ap, int id, bool looping)
{
	return play(ap, id, true, looping, 0.0f, 0.0f);
}

// stero (for in-world sounds)
int win::audio_engine::play(apack &ap, int id, float x, float y, bool looping)
{
	return play(ap, id, false, looping, x, y);
}

int win::audio_engine::play(apack &ap, int id, bool ambient, bool looping, float x, float y)
{
	if(id >= (int)ap.count_ || id < 0)
		bug("Apack id out of bounds");

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

	sound &stored = sounds_.emplace_front(sid, looping, 0, ap.stored_[id].buffer.get(), &ap.stored_[id].size, ap.stored_[id].target_size, stream, ambient, x, y);

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
	pa_threaded_mainloop_lock(loop_);
	for(sound &snd : sounds_)
		pa_operation_unref(pa_stream_cork(snd.stream, 1, NULL, NULL));

	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::resume()
{
	pa_threaded_mainloop_lock(loop_);

	for(sound &snd : sounds_)
		pa_operation_unref(pa_stream_cork(snd.stream, 0, NULL, NULL));

	pa_threaded_mainloop_unlock(loop_);
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
	pa_threaded_mainloop_lock(rhs.loop_);
	rhs.cleanup(true);
	pa_threaded_mainloop_unlock(rhs.loop_);

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

		// prevent pulseaudio from being a bastard
		pa_stream_set_state_callback(snd.stream, [](pa_stream*, void*){}, NULL);
		pa_stream_set_write_callback(snd.stream, [](pa_stream*, size_t, void*){}, NULL);

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

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

static constexpr unsigned long long SOUND_BUFFER_SIZE = 6 * 44100 * sizeof(short); // seconds * sample rate * sample size
static constexpr unsigned long long MAX_WRITE_SIZE = SOUND_BUFFER_SIZE / 2;

static void write_buffer(win::sound &snd, const DWORD offset, const int bytes)
{
	void *buffer1 = NULL;
	void *buffer2 = NULL;
	DWORD size1 = 0;
	DWORD size2 = 0;

	if(snd.stream->Lock(offset, bytes, &buffer1, &size1, &buffer2, &size2, 0) != DS_OK)
		throw win::exception("DirectSound: Couldn't lock buffer for writing");

	if(size1 + size2 != bytes)
		throw win::exception("DirectSound: Requested write = " + std::to_string(bytes) + ", actual write = " + std::to_string(size1 + size2));

	memcpy(buffer1, (char*)snd.pcm + snd.start, size1);
	memcpy(buffer2, (char*)snd.pcm + snd.start + size1, size2);

	if(snd.stream->Unlock(buffer1, size1, buffer2, size2) != DS_OK)
		throw win::exception("DirectSound: Couldn't unlock buffer after writing");
}

static void write_directsound(win::sound &snd)
{
	if(snd.start == snd.target_size)
		return;

	const int size = snd.size->load();
	const int want_to_write = std::min(size - snd.start, MAX_WRITE_SIZE);
	const int offset = snd.write_cursor;

	if(offset >= SOUND_BUFFER_SIZE)
		throw win::exception("offset too big");

	write_buffer(snd, offset, want_to_write);

	snd.start += want_to_write;
	snd.write_cursor = (snd.write_cursor + want_to_write) % SOUND_BUFFER_SIZE;
}

win::audio_engine::audio_engine()
{
	next_id_ = 1;
	listener_x_ = 0.0f;
	listener_y_ = 0.0f;
	config_fn_ = default_sound_config_fn;

	parent_ = NULL;
	context_ = NULL;
	primary_ = NULL;
	last_poke_ = std::chrono::high_resolution_clock::now();
}

win::audio_engine::audio_engine(sound_config_fn fn, display *parent)
{
	parent->directsound_ = this;
	parent_ = parent;
	last_poke_ = std::chrono::high_resolution_clock::now();
	next_id_ = 1;
	listener_x_ = 0.0f;
	listener_y_ = 0.0f;
	config_fn_ = fn;

	if(DirectSoundCreate8(NULL, &context_, NULL) != DS_OK)
		throw exception("Could not initialize DirectSound");

	if(context_->SetCooperativeLevel(parent->window_, DSSCL_PRIORITY) != DS_OK)
		throw exception("DirectSound: Could not set cooperation level");

	DSBUFFERDESC buffer;
	buffer.dwSize = sizeof(buffer);
	buffer.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	buffer.dwBufferBytes = 0;
	buffer.dwReserved = 0;
	buffer.lpwfxFormat = NULL;
	buffer.guid3DAlgorithm = GUID_NULL;

	if(context_->CreateSoundBuffer(&buffer, &primary_, NULL) != DS_OK)
		throw exception("DirectSound: Could not create the primary sound buffer");

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 44100 * 2;
	format.nBlockAlign = 2;
	format.wBitsPerSample = 16;
	format.cbSize = 0;

	if(primary_->SetFormat(&format) != DS_OK)
		throw exception("DirectSound: Could not set the primary buffer format");
}

int win::audio_engine::play(apack &ap, int id, bool loop)
{
	return play(ap, id, true, loop, 0.0f, 0.0f);
}

int win::audio_engine::play(apack &ap, int id, float x, float y, bool loop)
{
	return play(ap, id, false, loop, x, y);
}

int win::audio_engine::play(apack &ap, int id, bool ambient, bool looping, float x, float y)
{
	if(id >= ap.count_ || id < 0)
		throw exception("Invalid apack index " + std::to_string(id));

	if(sounds_.size() >= MAX_SOUNDS)
		return -1;

	const unsigned long long size = ap.stored_[id].size.load();

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
	if(context_->CreateSoundBuffer(&buffer, &tmp, NULL) != DS_OK)
		throw exception("DirectSound: Could not create temp buffer");

	IDirectSoundBuffer8 *stream;
	tmp->QueryInterface(IID_IDirectSoundBuffer8, (void**)&stream);
	tmp->Release();

	sound &snd = sounds_.emplace_back(next_id_++, looping, 0, ap.stored_[id].buffer.get(), &ap.stored_[id].size, ap.stored_[id].target_size, ambient, x, y, stream);

	write_directsound(snd);

	snd.stream->Play(0, 0, DSBPLAY_LOOPING);

	return snd.id;
}

void win::audio_engine::pause()
{
}

void win::audio_engine::resume()
{
}

void win::audio_engine::pause(int)
{
}

void win::audio_engine::resume(int)
{
}

void win::audio_engine::source(int, float, float)
{
}

void win::audio_engine::listener(float x, float y)
{
	listener_x_ = x;
	listener_y_ = y;
}

// move platform-specific (DirectSound) data members
void win::audio_engine::move_platform(audio_engine &rhs)
{
	if(parent_ != NULL)
		parent_->directsound_ = NULL;
	parent_ = rhs.parent_;
	rhs.parent_ = NULL;
	parent_->directsound_ = this;

	context_ = rhs.context_;
	rhs.context_ = NULL;

	primary_ = rhs.primary_;
	sounds_ = std::move(rhs.sounds_);

	last_poke_ = std::move(rhs.last_poke_);
}

void win::audio_engine::finalize()
{
	if(context_ == NULL)
		return;

	cleanup();
	primary_->Release();
	context_->Release();

	context_ = NULL;
}

void win::audio_engine::poke()
{
	const auto now = std::chrono::high_resolution_clock::now();

	if(std::chrono::duration<double, std::milli>(now - last_poke_).count() < 10)
		return;

	last_poke_ = now;

	for(auto snd = sounds_.begin(); snd != sounds_.end();)
	{
		DWORD play_cursor = 0;
		if(snd->stream->GetCurrentPosition(&play_cursor, NULL) != DS_OK)
			throw exception("DirectSound: Couldn't determine play cursor position");

		const int write_cursor = snd->write_cursor < play_cursor ? (snd->write_cursor + SOUND_BUFFER_SIZE) : snd->write_cursor;
		const int bytes_left = write_cursor - play_cursor;
		if(bytes_left < 44100 * 2)
			write_directsound(*snd);
		if(bytes_left > MAX_WRITE_SIZE && snd->start == snd->target_size)
		{
			snd->finalize();
			snd = sounds_.erase(snd);
			continue;
		}

		++snd;
	}

	const double dur = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - now).count();
	if(dur > 5)
		std::cerr << "took longer than 5 milliseconds" << std::endl;
}

void win::audio_engine::cleanup()
{
	for(auto snd = sounds_.begin(); snd != sounds_.end();)
	{
		snd->finalize();
		snd = sounds_.erase(snd);
	}
}

#endif
