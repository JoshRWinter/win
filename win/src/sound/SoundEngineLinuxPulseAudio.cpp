#include <win/Win.hpp>

#if defined WINPLAT_LINUX

#include <dlfcn.h>

#include <win/sound/SoundEngineLinuxPulseAudio.hpp>

static void underflow(pa_stream*, void*)
{
	fprintf(stderr, "PulseAudio: underflow\n");
}

namespace win
{

// function pointers
static pa_threaded_mainloop* (*pa_threaded_mainloop_new)();
static pa_mainloop_api* (*pa_threaded_mainloop_get_api)(pa_threaded_mainloop*);
static void (*pa_threaded_mainloop_lock)(pa_threaded_mainloop *loop);
static void (*pa_threaded_mainloop_unlock)(pa_threaded_mainloop *loop);
static int (*pa_threaded_mainloop_start)(pa_threaded_mainloop *loop);
static void (*pa_threaded_mainloop_stop)(pa_threaded_mainloop *loop);
static void (*pa_threaded_mainloop_free)(pa_threaded_mainloop *loop);

static pa_context* (*pa_context_new)(pa_mainloop_api *loop, const char *name);
static pa_context_state_t (*pa_context_get_state)(const pa_context *context);
static void (*pa_context_disconnect)(pa_context *context);
static int (*pa_context_connect)(pa_context *context, const char *server, pa_context_flags_t flags, const pa_spawn_api *api);
static void (*pa_context_unref)(pa_context *context);

static pa_stream* (*pa_stream_new)(const pa_context *context, const char *name, const pa_sample_spec *ss, const pa_channel_map *map);
static int (*pa_stream_connect_playback)(const pa_stream *stream, const char *dev, const pa_buffer_attr *attr, pa_stream_flags_t flags, const pa_cvolume *volume, pa_stream *sync_stream);
static void (*pa_stream_set_write_callback)(pa_stream *stream, pa_stream_request_cb_t callback, void *userdata);
static void (*pa_stream_set_underflow_callback)(pa_stream *stream, pa_stream_notify_cb_t callback, void *userdata);
static int (*pa_stream_begin_write)(pa_stream *stream, void **data, size_t *nbytes);
static int (*pa_stream_write)(pa_stream *stream, const void *data, size_t nbytes, pa_free_cb_t free_cb, int64_t offset, pa_seek_mode_t seek);
static pa_stream_state_t (*pa_stream_get_state)(const pa_stream *stream);
static int (*pa_stream_disconnect)(pa_stream *stream);
static void (*pa_stream_unref)(pa_stream *stream);
static pa_operation *(*pa_stream_cork)(pa_stream*, int, pa_stream_success_cb_t, void*);

void (*pa_operation_unref)(pa_operation *op);

SoundEngineLinuxPulseAudio::SoundEngineLinuxPulseAudio(AssetRoll &asset_roll, const char *soname)
	: mixer(asset_roll)
{
	// initialize dll

	so = dlopen(soname, RTLD_LAZY);
	if (so == NULL)
		win::bug("Couldn't load " + std::string(soname));

	load_functions();

	// set up the loop and context

	loop = pa_threaded_mainloop_new();
	if (loop == NULL)
		win::bug("Could not initialize process loop");

	if (pa_threaded_mainloop_start(loop))
		win::bug("PulseAudio: Couldn't start the process loop");

	pa_threaded_mainloop_lock(loop);

	context = pa_context_new(pa_threaded_mainloop_get_api(loop), "wet fart");
	if (context == NULL)
		win::bug("PulseAudio: Couldn't create context");

	if (pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL))
		win::bug("PulseAudio: Couldn't connect the context");

	for (;;)
	{
		const pa_context_state_t context_state = pa_context_get_state(context);
		if (context_state == PA_CONTEXT_READY)
			break;
		else if (context_state == PA_CONTEXT_FAILED)
			win::bug("PulseAudio: Context connection failed");

		pa_threaded_mainloop_unlock(loop);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pa_threaded_mainloop_lock(loop);
	}

	// set up the stream

	pa_sample_spec spec;
	spec.format = PA_SAMPLE_S16LE;
	spec.channels = 2;
	spec.rate = 44100;

	stream = pa_stream_new(context, "wet fart", &spec, NULL);
	if(stream == NULL)
		win::bug("PulseAudio: Couldn't create stream");

	pa_stream_set_write_callback(stream, process, this);
	pa_stream_set_underflow_callback(stream, underflow, NULL);

	const int desired_latency = 180; // 4 millis
	const int pulseaudio_is_lame_latency = desired_latency * 6;
	const int pulseaudio_is_lame_latency_stereo = pulseaudio_is_lame_latency * 2;

	pa_buffer_attr attr;
	attr.maxlength = pulseaudio_is_lame_latency_stereo * 3;
	attr.tlength = pulseaudio_is_lame_latency;
	attr.prebuf = 0;
	attr.minreq = (std::uint32_t) -1;
	attr.fragsize = (std::uint32_t) -1;

	if (pa_stream_connect_playback(stream, NULL, &attr, PA_STREAM_START_CORKED, NULL, NULL))
		win::bug("PulseAudio: Couldn't connect stream");

	for (;;)
	{
		const pa_stream_state_t state = pa_stream_get_state(stream);
		if (state == PA_STREAM_READY)
			break;
		else if (state == PA_STREAM_FAILED)
			win::bug("PulseAudio: Couldn't connect stream");

		pa_threaded_mainloop_unlock(loop);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pa_threaded_mainloop_lock(loop);
	}

	pa_operation_unref(pa_stream_cork(stream, 0, NULL, NULL));

	pa_threaded_mainloop_unlock(loop);
}

SoundEngineLinuxPulseAudio::~SoundEngineLinuxPulseAudio()
{
	pa_threaded_mainloop_lock(loop);

	if (pa_stream_disconnect(stream))
		win::bug("PulseAudio: Couldn't disconnect stream");

	while (true)
	{
		const pa_stream_state_t state = pa_stream_get_state(stream);

		if (state == PA_STREAM_TERMINATED)
			break;

		pa_threaded_mainloop_unlock(loop);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pa_threaded_mainloop_lock(loop);
	}

	pa_stream_unref(stream);


	pa_context_disconnect(context);
	pa_context_unref(context);

	pa_threaded_mainloop_unlock(loop);
	pa_threaded_mainloop_stop(loop);
	pa_threaded_mainloop_free(loop);

	if (dlclose(so))
		win::bug("dlclose() failed");
}

std::uint32_t SoundEngineLinuxPulseAudio::play(const SoundEnginePlayCommand &cmd)
{
	pa_threaded_mainloop_lock(loop);
	const auto key = mixer.add(cmd.name, cmd.residency_priority, cmd.compression_priority, cmd.left, cmd.right, cmd.looping, cmd.cache, cmd.seek);
	pa_threaded_mainloop_unlock(loop);

	return key;
}

void SoundEngineLinuxPulseAudio::save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &config)
{
	pa_threaded_mainloop_lock(loop);

	for (const auto &cmd : playback)
	{
		if (cmd.stop)
			mixer.stop(cmd.key);
		else if (cmd.playing)
			mixer.resume(cmd.key);
		else
			mixer.pause(cmd.key);
	}

	for (const auto &cmd : config)
	{
		mixer.config(cmd.key, cmd.left, cmd.right);
	}

	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::process(pa_stream *stream, const size_t request_bytes, void *userdata)
{
	SoundEngineLinuxPulseAudio &engine = *(SoundEngineLinuxPulseAudio*)userdata;

	size_t sent_bytes = 0;

	for (int i = 0; i < 5 && sent_bytes < request_bytes; ++i)
	{
		std::int16_t *dest = NULL;
		size_t give_bytes = request_bytes - sent_bytes;
		if (pa_stream_begin_write(stream, (void**)&dest, &give_bytes))
			win::bug("PulseAudio: begin write failed");

		const size_t give_samples = give_bytes / 2;

		const int got_samples = engine.mixer.mix_stereo(dest, (int)give_samples);
		const int gave_bytes = got_samples * 2;

		if (pa_stream_write(stream, dest, gave_bytes, NULL, 0, PA_SEEK_RELATIVE))
			win::bug("PulseAudio: write failed");

		sent_bytes += gave_bytes;
	}
}

static void *load_pa_fn(const char*, void *so);

void SoundEngineLinuxPulseAudio::load_functions()
{
	pa_threaded_mainloop_new = (decltype(pa_threaded_mainloop_new)) load_pa_fn("pa_threaded_mainloop_new", so);
	pa_threaded_mainloop_get_api = (decltype(pa_threaded_mainloop_get_api)) load_pa_fn("pa_threaded_mainloop_get_api",
																					   so);
	pa_threaded_mainloop_lock = (decltype(pa_threaded_mainloop_lock)) load_pa_fn("pa_threaded_mainloop_lock", so);
	pa_threaded_mainloop_unlock = (decltype(pa_threaded_mainloop_unlock)) load_pa_fn("pa_threaded_mainloop_unlock", so);
	pa_threaded_mainloop_start = (decltype(pa_threaded_mainloop_start)) load_pa_fn("pa_threaded_mainloop_start", so);
	pa_threaded_mainloop_stop = (decltype(pa_threaded_mainloop_stop)) load_pa_fn("pa_threaded_mainloop_stop", so);
	pa_threaded_mainloop_free = (decltype(pa_threaded_mainloop_free)) load_pa_fn("pa_threaded_mainloop_free", so);

	pa_context_new = (decltype(pa_context_new)) load_pa_fn("pa_context_new", so);
	pa_context_get_state = (decltype(pa_context_get_state)) load_pa_fn("pa_context_get_state", so);
	pa_context_disconnect = (decltype(pa_context_disconnect)) load_pa_fn("pa_context_disconnect", so);
	pa_context_connect = (decltype(pa_context_connect)) load_pa_fn("pa_context_connect", so);
	pa_context_unref = (decltype(pa_context_unref)) load_pa_fn("pa_context_unref", so);

	pa_stream_new = (decltype(pa_stream_new)) load_pa_fn("pa_stream_new", so);
	pa_stream_connect_playback = (decltype(pa_stream_connect_playback)) load_pa_fn("pa_stream_connect_playback", so);
	pa_stream_get_state = (decltype(pa_stream_get_state)) load_pa_fn("pa_stream_get_state", so);
	pa_stream_disconnect = (decltype(pa_stream_disconnect)) load_pa_fn("pa_stream_disconnect", so);
	pa_stream_set_write_callback = (decltype(pa_stream_set_write_callback)) load_pa_fn("pa_stream_set_write_callback", so);
	pa_stream_set_underflow_callback = (decltype(pa_stream_set_underflow_callback)) load_pa_fn("pa_stream_set_underflow_callback", so);
	pa_stream_begin_write = (decltype(pa_stream_begin_write)) load_pa_fn("pa_stream_begin_write", so);
	pa_stream_write = (decltype(pa_stream_write)) load_pa_fn("pa_stream_write", so);
	pa_stream_unref = (decltype(pa_stream_unref)) load_pa_fn("pa_stream_unref", so);
	pa_stream_cork = (decltype(pa_stream_cork)) load_pa_fn("pa_stream_cork", so);

	pa_operation_unref = (decltype(pa_operation_unref)) load_pa_fn("pa_operation_unref", so);
}

static void* load_pa_fn(const char *name, void *so)
{
	void *fn = dlsym(so, name);

	if (fn == NULL)
		win::bug("PulseAudio: Couldn't load function " + std::string(name));

	return fn;
}

}

#endif
