#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <dlfcn.h>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>

#include <win/sound/SoundEngineLinuxPipeWire.hpp>

namespace win
{

static void (*pw_init)(int *argc, char ***argv);
static void (*pw_deinit)();

static pw_thread_loop *(*pw_thread_loop_new)(const char *name, const spa_dict *props);
static int (*pw_thread_loop_start)(pw_thread_loop *loop);
static pw_loop *(*pw_thread_loop_get_loop)(pw_thread_loop *loop);
static void (*pw_thread_loop_lock)(pw_thread_loop *loop);
static void (*pw_thread_loop_unlock)(pw_thread_loop *loop);
static void (*pw_thread_loop_stop)(pw_thread_loop *loop);
static void (*pw_thread_loop_destroy)(pw_thread_loop *loop);

static pw_stream *(*pw_stream_new_simple)(pw_loop *loop, const char *name, pw_properties *props, pw_stream_events *events, void *data);
static int (*pw_stream_connect)(pw_stream *stream, spa_direction direction, uint32_t target_id, pw_stream_flags flags, const spa_pod **params, uint32_t nparams);
static pw_stream_state (*pw_stream_get_state)(pw_stream *stream, const char **error);
static int (*pw_stream_disconnect)(pw_stream *stream);
static void (*pw_stream_destroy)(pw_stream *stream);
static void (*pw_stream_set_active)(pw_stream *stream, int active);

static pw_buffer *(*pw_stream_dequeue_buffer)(pw_stream *stream);
static int (*pw_stream_queue_buffer)(pw_stream *stream, pw_buffer *buffer);

static pw_properties *(*pw_properties_new)(const char *key, ...);

SoundEngineLinuxPipeWire::SoundEngineLinuxPipeWire(AssetRoll &roll, const char *soname)
	: mixer(roll)
{
	so = dlopen(soname, RTLD_LAZY);
	if (so == NULL)
		win::bug("Couldn't load " + std::string(soname));

	load_functions();

	pw_init(NULL, NULL);

	loop = pw_thread_loop_new("threadloop_lol", NULL);

	pw_thread_loop_start(loop);
	pw_thread_loop_lock(loop);

	memset(&events, 0, sizeof(events));
	events.version = PW_VERSION_STREAM_EVENTS;
	events.process = stream_process;

	stream = pw_stream_new_simple(
		pw_thread_loop_get_loop(loop),
		"wet fart",
		pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Game", NULL),
		&events,
		this
	);

	if (stream == NULL)
		win::bug("PipeWire: Couldn't create stream");

	spa_audio_info_raw info;
	memset(&info, 0, sizeof(info));
	info.format = SPA_AUDIO_FORMAT_S16;
	info.channels = 2;
	info.rate = 44100;

	unsigned char pod[1024];
	spa_pod_builder builder;
	spa_pod_builder_init(&builder, pod, sizeof(pod));
	const spa_pod *params = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &info);

	const char *stuff;
	const auto shit = pw_stream_get_state(stream, &stuff);

	if (pw_stream_connect(stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, (pw_stream_flags)(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS), &params, 1) != 0)
		win::bug("PipeWire: Couldn't connect stream");

	const char *errortext;
	while (true)
	{
		pw_thread_loop_unlock(loop);
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
		pw_thread_loop_lock(loop);

		if (pw_stream_get_state(stream, &errortext) == PW_STREAM_STATE_STREAMING)
			break;
	}

	pw_thread_loop_unlock(loop);
}

SoundEngineLinuxPipeWire::~SoundEngineLinuxPipeWire()
{
	pw_thread_loop_lock(loop);

	pw_stream_disconnect(stream);
	pw_stream_destroy(stream);

	pw_thread_loop_unlock(loop);
	pw_thread_loop_stop(loop);
	pw_thread_loop_destroy(loop);

	pw_deinit();

	if (dlclose(so))
		win::bug("dlclose() failed");
}

std::uint32_t SoundEngineLinuxPipeWire::play(const SoundEnginePlayCommand &cmd)
{
	pw_thread_loop_lock(loop);
	const auto key = mixer.add(cmd.name, cmd.residency_priority, cmd.compression_priority, cmd.left, cmd.right, cmd.looping, cmd.cache, cmd.seek);
	pw_thread_loop_unlock(loop);

	return key;
}

void SoundEngineLinuxPipeWire::save(const std::vector<SoundEnginePlaybackCommand> &playback, const std::vector<SoundEngineConfigCommand> &config)
{
	pw_thread_loop_lock(loop);

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

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::stream_process(void *userdata)
{
	SoundEngineLinuxPipeWire &engine = *(SoundEngineLinuxPipeWire*)userdata;

	pw_buffer *pwbuffer = pw_stream_dequeue_buffer(engine.stream);
	if (pwbuffer == NULL)
		win::bug("PipeWire: Couldn't dequeue buffer");

	spa_buffer *buffer = pwbuffer->buffer;
	if (buffer->datas[0].data == NULL)
		return;

	if (buffer->n_datas > 0)
	{
		const auto pw_wants_samples = buffer->datas[0].maxsize / sizeof(std::int16_t);
		const auto got_samples = engine.mixer.mix_stereo((std::int16_t*)buffer->datas[0].data, (int)pw_wants_samples);

		buffer->datas[0].chunk->offset = 0;
		buffer->datas[0].chunk->stride = sizeof(std::int16_t) * 2;
		buffer->datas[0].chunk->size = got_samples * sizeof(std::int16_t);
	}

	if (pw_stream_queue_buffer(engine.stream, pwbuffer))
		win::bug("PipeWire: Couldn't queue buffer");
}

static void *load_fn(void*, const char*);
void SoundEngineLinuxPipeWire::load_functions()
{

	pw_init = (decltype(pw_init))load_fn(so, "pw_init");
	pw_deinit = (decltype(pw_deinit))load_fn(so, "pw_deinit");

	pw_thread_loop_new = (decltype(pw_thread_loop_new))load_fn(so, "pw_thread_loop_new");
	pw_thread_loop_get_loop = (decltype(pw_thread_loop_get_loop))load_fn(so, "pw_thread_loop_get_loop");
	pw_thread_loop_start = (decltype(pw_thread_loop_start))load_fn(so, "pw_thread_loop_start");
	pw_thread_loop_lock = (decltype(pw_thread_loop_lock))load_fn(so, "pw_thread_loop_lock");
	pw_thread_loop_unlock = (decltype(pw_thread_loop_unlock))load_fn(so, "pw_thread_loop_unlock");
	pw_thread_loop_stop = (decltype(pw_thread_loop_stop))load_fn(so, "pw_thread_loop_stop");
	pw_thread_loop_destroy = (decltype(pw_thread_loop_destroy))load_fn(so, "pw_thread_loop_destroy");

	pw_stream_new_simple = (decltype(pw_stream_new_simple))load_fn(so, "pw_stream_new_simple");
	pw_stream_connect = (decltype(pw_stream_connect))load_fn(so, "pw_stream_connect");
	pw_stream_get_state = (decltype(pw_stream_get_state))load_fn(so, "pw_stream_get_state");
	pw_stream_disconnect = (decltype(pw_stream_disconnect))load_fn(so, "pw_stream_disconnect");
	pw_stream_destroy = (decltype(pw_stream_destroy))load_fn(so, "pw_stream_destroy");
	pw_stream_set_active = (decltype(pw_stream_set_active))load_fn(so, "pw_stream_set_active");

	pw_stream_dequeue_buffer = (decltype(pw_stream_dequeue_buffer))load_fn(so, "pw_stream_dequeue_buffer");
	pw_stream_queue_buffer = (decltype(pw_stream_queue_buffer))load_fn(so, "pw_stream_queue_buffer");

	pw_properties_new = (decltype(pw_properties_new))load_fn(so, "pw_properties_new");
}

void *load_fn(void *so, const char *name)
{
	void *fn = dlsym(so, name);

	if (fn == NULL)
		win::bug("PipeWire: Couldn't load function " + std::string(name));

	return fn;
}

}

#endif
