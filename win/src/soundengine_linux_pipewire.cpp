#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>

#include <win/soundengine_linux_pipewire.hpp>


namespace win
{

SoundEngineLinuxPipeWire::SoundEngineLinuxPipeWire(AssetRoll &roll)
	: mixer(roll)
{
	pw_init(0, NULL);

	//loop = pw_main_loop_new(NULL);
	loop = pw_thread_loop_new("threadloop_lol", NULL);
/*
	context = pw_context_new(pw_thread_loop_get_loop(loop), NULL, 0);
	core = pw_context_connect(context, NULL, 0);
	registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
*/

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

	if (pw_stream_connect(stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, (pw_stream_flags)(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS), &params, 1) != 0)
		win::bug("PipeWire: Couldn't connect stream");

	pw_thread_loop_unlock(loop);
	const char *errortext;
	while (1)
	{
		pw_thread_loop_unlock(loop);
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
		pw_thread_loop_lock(loop);

		if (pw_stream_get_state(stream, &errortext) == PW_STREAM_STATE_STREAMING)
			break;
	}

	pw_thread_loop_unlock(loop);
}

SoundEngineLinuxPipeWire::~SoundEngineLinuxPipeWire()
{
	pw_thread_loop_stop(loop);

	/*
	pw_core_disconnect(core);
	pw_context_destroy(context);
	*/

	pw_stream_disconnect(stream);
	pw_stream_destroy(stream);
	pw_thread_loop_destroy(loop);

	pw_deinit();
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
		const auto got_samples = engine.mixer.mix_stereo((std::int16_t*)buffer->datas[0].data, pw_wants_samples);

		buffer->datas[0].chunk->offset = 0;
		buffer->datas[0].chunk->stride = sizeof(std::int16_t) * 2;
		buffer->datas[0].chunk->size = got_samples * sizeof(std::int16_t);
	}

	if (pw_stream_queue_buffer(engine.stream, pwbuffer))
		win::bug("PipeWire: Couldn't queue buffer");
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, win::SoundResidencyPriority priority, float compression_priority, bool looping, int seek)
{
	return play(path, priority, compression_priority, 1.0f, 1.0f, looping, seek);
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, win::SoundResidencyPriority priority, float compression_priority, float left, float right, bool looping, int seek)
{
	pw_thread_loop_lock(loop);

	std::uint32_t key = mixer.add(path, priority, compression_priority, left, right, looping, seek);

	pw_thread_loop_unlock(loop);

	return key;
}

void SoundEngineLinuxPipeWire::pause(std::uint32_t id)
{
	pw_thread_loop_lock(loop);

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::resume(std::uint32_t id)
{
	pw_thread_loop_lock(loop);

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::config(std::uint32_t id, float pan, float volume)
{
	pw_thread_loop_lock(loop);

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::cleanup(bool force_stop)
{
	pw_thread_loop_lock(loop);

	pw_thread_loop_unlock(loop);
}

}

#endif
