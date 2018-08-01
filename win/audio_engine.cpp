#include "win.h"

win::audio_engine::audio_engine(audio_engine &&rhs)
{
	next_id_ = rhs.next_id_;

	move(rhs);
}

win::audio_engine::~audio_engine()
{
	finalize();
}

win::audio_engine &win::audio_engine::operator=(audio_engine &&rhs)
{
	finalize();

	next_id_ = rhs.next_id_;

	move(rhs);

	return *this;
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

static void callback_dummy_free(void*) {}
static void callback_stream_success(pa_stream*, int, void*) {}

static void callback_stream_drain(pa_stream *stream, int, void *data)
{
	((win::sound*)data)->done = true;
}

static void callback_stream_write(pa_stream *stream, size_t bytes, void *data)
{
	win::sound *const snd = (win::sound*)data;

	if(snd->start == (long long)snd->target_size)
		return;

	if(snd->cancel)
	{
		snd->start = snd->target_size;
		pa_operation_unref(pa_stream_drain(stream, callback_stream_drain, data));
	}

	const long long left = snd->size->load() - snd->start;
	const long long start = snd->start;
	const size_t written = std::min((long long)bytes, left);

	snd->start += written;

	pa_stream_write(stream, (char*)(snd->pcm) + start, written, callback_dummy_free, 0, PA_SEEK_RELATIVE);

	if(snd->start == (long long)snd->target_size)
		pa_operation_unref(pa_stream_drain(stream, callback_stream_drain, data));
}

win::audio_engine::audio_engine()
{
	next_id_ = 1;
	active_sounds_ = 0;

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

void win::audio_engine::play(const apack &ap, int id)
{
	if(active_sounds_ > MAX_SOUNDS)
	{
		pa_threaded_mainloop_lock(loop_);
		cleanup(false);
		pa_threaded_mainloop_unlock(loop_);
	}

	if(active_sounds_ > MAX_SOUNDS)
		return;

	++active_sounds_;

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

	sound snd;

	pa_threaded_mainloop_lock(loop_);

	// cleanup dead streams
	cleanup(false);

	snd.parent = this;
	snd.id = sid;
	snd.start = 0;
	snd.pcm = ap.sounds_[id].buffer.get();
	snd.size = &ap.sounds_[id].size;
	snd.target_size = ap.sounds_[id].target_size;
	snd.done = false;
	snd.cancel = false;
	snd.stream = pa_stream_new(context_, namestr, &spec, NULL);
	if(snd.stream == NULL)
		raise("Could not create stream object");

	sound *stored = sounds_.add(snd);

	pa_stream_set_state_callback(snd.stream, callback_stream, loop_);
	pa_stream_set_write_callback(snd.stream, callback_stream_write, stored);

	if(pa_stream_connect_playback(snd.stream, NULL, &attr, (pa_stream_flags)flags, NULL, NULL))
		raise("Could not connect the playback stream");

	for(;;)
	{
		pa_stream_state_t state = pa_stream_get_state(snd.stream);
		if(state == PA_STREAM_READY)
			break;
		else if(state == PA_STREAM_FAILED)
			raise("Stream connection failed");
		pa_threaded_mainloop_wait(loop_);
	}

	pa_operation_unref(pa_stream_cork(snd.stream, 0, callback_stream_success, loop_));
	while(pa_stream_is_corked(snd.stream));
	pa_threaded_mainloop_unlock(loop_);
}

void win::audio_engine::pause_all()
{
	auto callback = [](pa_stream*, int, void*) {};
	sound_list::node *current = sounds_.head;
	while(current != NULL)
	{
		pa_operation *op = pa_stream_cork(current->snd.stream, 1, callback, NULL);
		if(op)
		{
			while(pa_operation_get_state(op) != PA_OPERATION_DONE);
			pa_operation_unref(op);
		}

		current = current->next;
	}
}

void win::audio_engine::resume_all()
{
	auto callback = [](pa_stream*, int, void*) {};

	sound_list::node *current = sounds_.head;
	while(current != NULL)
	{
		pa_operation *op = pa_stream_cork(current->snd.stream, 0, callback, NULL);
		if(op)
		{
			while(pa_operation_get_state(op) != PA_OPERATION_DONE);
			pa_operation_unref(op);
		}

		current = current->next;
	}
}

void win::audio_engine::move(audio_engine &rhs)
{
	sounds_ = std::move(rhs.sounds_);

	context_ = rhs.context_;
	loop_ = rhs.loop_;
	active_sounds_ = rhs.active_sounds_;

	rhs.context_ = NULL;
	rhs.loop_ = NULL;
}

void win::audio_engine::cleanup(bool all)
{
	sound_list::node *current = sounds_.head;
	while(current != NULL)
	{
		sound &snd = current->snd;

		if(!all)
			if(!snd.done)
			{
				current = current->next;
				continue;
			}

		snd.cancel = true;

		if(!snd.done)
		{
			pa_threaded_mainloop_unlock(loop_);
			pa_operation *op = pa_stream_flush(snd.stream, [](pa_stream*,int,void*){}, NULL);
			if(op)
			{
				while(pa_operation_get_state(op) != PA_OPERATION_DONE);
				pa_operation_unref(op);
			}
			pa_threaded_mainloop_lock(loop_);
		}

		pa_threaded_mainloop_unlock(loop_);
		while(!snd.done);
		pa_threaded_mainloop_lock(loop_);

		if(pa_stream_disconnect(snd.stream))
			raise("Couldn't disconnect stream");
		pa_stream_unref(snd.stream);
		--active_sounds_;

		current = sounds_.remove(current);
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
