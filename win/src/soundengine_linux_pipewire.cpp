
#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include <win/soundengine_linux_pipewire.hpp>

namespace win
{


SoundEngineLinuxPipeWire::SoundEngineLinuxPipeWire(/*Display &p, */AssetRoll &asset_roll, SoundConfigFn fn)
	: soundbank(asset_roll)
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
}

SoundEngineLinuxPipeWire::~SoundEngineLinuxPipeWire()
{
    pw_thread_loop_stop(loop);

	/*
	pw_core_disconnect(core);
	pw_context_destroy(context);
	*/
	pw_thread_loop_destroy(loop);
}

void SoundEngineLinuxPipeWire::stream_drained(void *userdata)
{
   	PipeWireActiveSound &sound = *(PipeWireActiveSound*)userdata;

	if (sound.stream == NULL)
		return;

    if (pw_stream_disconnect(sound.stream) != 0)
		win::bug("Pipewire: couldn't disconnect");

	// pipewire calls these callbacks with the "thread lock" held
	// no need for special syncronization with "main" thread
	sound.done = true;
}


void SoundEngineLinuxPipeWire::process_stream(void *userdata)
{
   	PipeWireActiveSound &sound = *(PipeWireActiveSound*)userdata;

	if (sound.stream == NULL)
		return;

    pw_buffer *pwbuffer = pw_stream_dequeue_buffer(sound.stream);
    if (pwbuffer == NULL)
	    win::bug("pipewire dun goofed");

    spa_buffer *buffer = pwbuffer->buffer;
    std::int16_t *dest = (std::int16_t*)buffer->datas[0].data;
    if (dest == NULL)
	    return;

	const int channels = sound.sound->channels.load();
	for (int i = 0; i < buffer->n_datas; ++i)
	{
		const auto pw_needs_samples = buffer->datas[i].maxsize / sizeof(std::int16_t);
		const auto got_samples = sound.sound->read((std::int16_t*)buffer->datas[i].data, pw_needs_samples);

		if (got_samples == 0)
			break; // ran out of decoded data, skip

		buffer->datas[i].chunk->offset = 0;
		buffer->datas[i].chunk->stride = sizeof(std::int16_t) * channels;
		buffer->datas[i].chunk->size = got_samples * sizeof(std::int16_t); // got_samples * 2 -- turn sample count back into byte count
	}

	if (!sound.flushing && sound.sound->is_stream_completed())
	{
		sound.flushing = true;
		pw_stream_flush(sound.stream, true);
	}

	pw_stream_queue_buffer(sound.stream, pwbuffer);
}

void SoundEngineLinuxPipeWire::process()
{
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, bool looping)
{
	return play(path, 0.0f, 0.0f, true, true);
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, float x, float y, bool looping)
{
	return play(path, x, y, false, true);
}

static pw_stream_events stream_events;

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, float x, float y, bool ambient, bool looping)
{
	pw_thread_loop_lock(loop);

    stream_events.version = PW_VERSION_STREAM_EVENTS;
	stream_events.process = process_stream;
	stream_events.drained = stream_drained;

	const std::uint32_t key = sounds.add(SoundPriority::high, soundbank.load(path));
	if (key == -1)
		return -1;

	PipeWireActiveSound *const sound_ptr = sounds[key];
	if (sound_ptr == NULL)
		return -1;
	PipeWireActiveSound &sound = *sound_ptr;

	while (sound.sound->channels.load() == -1); // wait for channels to load

	pw_stream *stream = pw_stream_new_simple(pw_thread_loop_get_loop(loop), "1", pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Music", NULL), &stream_events, &sound);
	if (stream == NULL)
		win::bug("no stream");
	sound.stream = stream;

	spa_audio_info_raw info;
	memset(&info, 0, sizeof(info));
	info.format = SPA_AUDIO_FORMAT_S16;
	info.channels = 2;
	info.rate = 44100;

	unsigned char pod[1024];
	spa_pod_builder builder;// = SPA_POD_BUILDER_INIT(pod, sizeof(pod));
	spa_pod_builder_init(&builder, pod, sizeof(pod));
	const spa_pod *params = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &info);
	if (pw_stream_connect(stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, (pw_stream_flags)(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS), &params, 1) != 0)
		win::bug("pw_stream_connect error");

	pw_thread_loop_unlock(loop);
	return 0;
}

void SoundEngineLinuxPipeWire::pause(std::uint32_t id)
{
}

void SoundEngineLinuxPipeWire::resume(std::uint32_t id)
{
}

void SoundEngineLinuxPipeWire::stop(std::uint32_t id)
{
}

void SoundEngineLinuxPipeWire::source(int id, float x, float y)
{
}

void SoundEngineLinuxPipeWire::listener(float x, float y)
{
}

void SoundEngineLinuxPipeWire::cleanup()
{
	pw_thread_loop_lock(loop);



	pw_thread_loop_unlock(loop);
}

}

#endif
