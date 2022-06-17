#include <win/win.hpp>

#ifdef WINPLAT_LINUX

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>

#include <win/soundengine_linux_pipewire.hpp>


namespace win
{

/*
SoundEngineLinuxPipeWire::SoundEngineLinuxPipeWire(/*Display &p, AssetRoll &asset_roll)
	: soundbank(asset_roll)
{
	pw_init(0, NULL);

	//loop = pw_main_loop_new(NULL);
	loop = pw_thread_loop_new("threadloop_lol", NULL);
	/*
	context = pw_context_new(pw_thread_loop_get_loop(loop), NULL, 0);
	core = pw_context_connect(context, NULL, 0);
	registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);

    pw_thread_loop_start(loop);
}

SoundEngineLinuxPipeWire::~SoundEngineLinuxPipeWire()
{
    pw_thread_loop_stop(loop);

	/*
	pw_core_disconnect(core);
	pw_context_destroy(context);

	cleanup(true);

	pw_thread_loop_destroy(loop);

	pw_deinit();
}

void SoundEngineLinuxPipeWire::stream_drained(void *userdata)
{
   	PipeWireActiveSound &sound = *(PipeWireActiveSound*)userdata;

    if (pw_stream_set_active(sound.stream, false))
		win::bug("PipeWire: Couldn't set inactive");

	sound.done = true;
}


void SoundEngineLinuxPipeWire::stream_process(void *userdata)
{
   	PipeWireActiveSound &sound = *(PipeWireActiveSound*)userdata;

	if (sound.sound->is_stream_completed())
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

	if (pw_stream_queue_buffer(sound.stream, pwbuffer))
		win::bug("PipeWire: Couldn't queue buffer");

	if (sound.sound->is_stream_completed())
		pw_stream_flush(sound.stream, true);
}

void SoundEngineLinuxPipeWire::stream_state_changed(void *userdata)
{
   	PipeWireActiveSound &sound = *(PipeWireActiveSound*)userdata;

	float values[2] = { sound.vol_left, sound.vol_right };
	if (!pw_stream_set_control(sound.stream, SPA_PROP_channelVolumes, 2, values, 0))
		win::bug("PipeWire: Couldn't set stream controls");
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, bool looping)
{
	return play(path, 0.0f, 0.0f, true, true);
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, float x, float y, bool looping)
{
	return play(path, x, y, false, true);
}

std::uint32_t SoundEngineLinuxPipeWire::play(const char *path, float x, float y, bool ambient, bool looping)
{
	pw_thread_loop_lock(loop);

	const std::uint32_t key = sounds.add(SoundPriority::high, soundbank.load(path));
	if (key == -1)
		return -1;

	PipeWireActiveSound *const sound_ptr = sounds[key];
	if (sound_ptr == NULL)
		return -1;
	PipeWireActiveSound &sound = *sound_ptr;

    sound.events.version = PW_VERSION_STREAM_EVENTS;
	sound.events.process = stream_process;
	sound.events.drained = stream_drained;

	pw_stream *stream = pw_stream_new_simple(pw_thread_loop_get_loop(loop), NULL, pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_MEDIA_CATEGORY, "Playback", PW_KEY_MEDIA_ROLE, "Game", NULL), &sound.events, &sound);
	if (stream == NULL)
		win::bug("Pipewire: Couldn't create stream");
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
		win::bug("PipeWire: Couldn't connect stream");

	pw_thread_loop_unlock(loop);
	return 0;
}

void SoundEngineLinuxPipeWire::pause(std::uint32_t id)
{
	pw_thread_loop_lock(loop);

	PipeWireActiveSound *sound = sounds[id];
    if (sound != NULL)
		if (pw_stream_set_active(sound->stream, false))
			win::bug("PipeWire: Couldn't set inactive");

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::resume(std::uint32_t id)
{
	pw_thread_loop_lock(loop);

	PipeWireActiveSound *sound = sounds[id];
    if (sound != NULL)
		if (pw_stream_set_active(sound->stream, true))
			win::bug("PipeWire: Couldn't set active");

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::stop(std::uint32_t id)
{
	pw_thread_loop_lock(loop);

	PipeWireActiveSound *sound = sounds[id];
    if (sound != NULL)
	{
		if (pw_stream_set_active(sound->stream, false))
			win::bug("PipeWire: Couldn't set inactive");

		sound->done = true; // let cleanup() take care of it eventually
	}

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::config(std::uint32_t id, float pan, float volume)
{
	return;
	/*
	pw_thread_loop_lock(loop);

	PipeWireActiveSound *sound = sounds[id];
    if (sound != NULL)
	{
		const char *error;
		const pw_stream_state state = pw_stream_get_state(sound->stream, &error);
		if (error != NULL)
			win::bug(error);

		//const float leftness =
		//float volumes[2] = { pan + 2.0f

		if (state == PW_STREAM_STATE_CONNECTING)
		{
			sound->vol_left =
		}

		if (pw_stream_set_control(sound->stream, SPA_PROP_channelVolumes, 2, volumes, 0))
			win::bug("PipeWire: Couldn't set pan/volume");
	}

	pw_thread_loop_unlock(loop);
}

void SoundEngineLinuxPipeWire::cleanup(bool force_stop)
{
	pw_thread_loop_lock(loop);

	for (auto it = sounds.begin(); it != sounds.end();)
	{
		PipeWireActiveSound &sound = *it;

		const bool kill = force_stop || sound.done || sounds.size() == decltype(sounds)::maxsize;

		if (kill)
		{
			// make sure pipewire doesn't try anything funny
			memset(&sound.events, 0, sizeof(sound.events));

			if (pw_stream_disconnect(sound.stream))
				win::bug("PipeWire: Couldn't disconnect stream");
			pw_stream_destroy(sound.stream);

			soundbank.unload(*sound.sound);
	    	it = sounds.remove(it);
			continue;
		}

		++it;
	}

	pw_thread_loop_unlock(loop);
}
*/

}

#endif
