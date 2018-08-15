#include "win.h"

win::audio_engine::audio_engine(audio_engine &&rhs)
{
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

// move the member data that is common to all platforms
void win::audio_engine::move_common(audio_engine &rhs)
{
	next_id_ = rhs.next_id_;
	listener_x_ = rhs.listener_x_;
	listener_y_ = rhs.listener_y_;
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

static void callback_stream_write(pa_stream *stream, size_t bytes, void *data)
{
	win::sound *const snd = (win::sound*)data;

	const unsigned long long size = snd->size->load();

	if(snd->start == snd->target_size)
		return;

	const unsigned long long left = size - snd->start;
	const size_t written = std::min((long long unsigned)bytes, left);

	pa_stream_write(stream, (char*)(snd->pcm) + snd->start, written, [](void*){}, 0, PA_SEEK_RELATIVE);

	if(snd->start + written == snd->target_size)
	{
		pa_operation *op = pa_stream_drain(stream, callback_stream_drained, data);
		if(!op)
			raise("Couldn't drain the stream");
		pa_operation_unref(op);
	}

	snd->start += written;
}

win::audio_engine::audio_engine()
{
	next_id_ = 1;
	listener_x_ = 0.0f;
	listener_y_ = 0.0f;

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

int win::audio_engine::play(const apack &ap, int id, bool looping)
{
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
	spec.channels = 1;
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

	sound &stored = sounds_.emplace_front(this, sid, looping, 0, ap.sounds_[id].buffer.get(), &ap.sounds_[id].size, ap.sounds_[id].target_size, stream);

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

	pa_operation_unref(pa_stream_cork(stream, 0, [](pa_stream*, int, void*){}, loop_));
	while(pa_stream_is_corked(stream));
	pa_threaded_mainloop_unlock(loop_);

	return sid;
}

void win::audio_engine::pause()
{
	auto callback = [](pa_stream*, int, void*) {};

	for(sound &snd : sounds_)
	{
		pa_operation *op = pa_stream_cork(snd.stream, 1, callback, NULL);
		if(!op)
			raise("Couldn't pause the stream");

		while(pa_operation_get_state(op) != PA_OPERATION_DONE);
		pa_operation_unref(op);
	}
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

void win::audio_engine::pause(int)
{
}

void win::audio_engine::resume(int)
{
}

void win::audio_engine::listener(float x, float y)
{
	listener_x_ = x;
	listener_y_ = y;

	// pa_threaded_mainloop_lock(loop_);
	// pa_threaded_mainloop_unlock(loop_);
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
}

#elif defined WINPLAT_WINDOWS

#endif
