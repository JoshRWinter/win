#include <win/win.hpp>

#if defined WINPLAT_LINUX

#include <win/sound/soundengine_linux_pulseaudio.hpp>
#include <win/sound/soundengine_linux_pulseaudio_functions.hpp>

static void underflow(pa_stream *p, void *ud)
{
	fprintf(stderr, "PulseAudio: underflow\n");
}

namespace win
{

SoundEngineLinuxPulseAudio::SoundEngineLinuxPulseAudio(AssetRoll &asset_roll)
	: mixer(asset_roll)
{
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
	const int pulseaudio_is_lame_latency = desired_latency * 1;
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
	pa_threaded_mainloop_stop(loop);

	if (pa_stream_disconnect(stream))
		win::bug("PulseAudio: Couldn't disconnect stream");

	if (pa_stream_get_state(stream) != PA_STREAM_UNCONNECTED)
		win::bug("still connected");

	pa_stream_unref(stream);

	pa_context_disconnect(context);
	pa_threaded_mainloop_free(loop);
	pa_context_unref(context);
}

void SoundEngineLinuxPulseAudio::process(pa_stream *stream, size_t request_bytes, void *userdata)
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

		const int got_samples = engine.mixer.mix_stereo(dest, give_samples);
		const int gave_bytes = got_samples * 2;

		if (pa_stream_write(stream, dest, gave_bytes, NULL, 0, PA_SEEK_RELATIVE))
			win::bug("PulseAudio: write failed");

		sent_bytes += gave_bytes;
	}
}

std::uint32_t SoundEngineLinuxPulseAudio::play(const char *name, win::SoundResidencyPriority residency_priority, float compression_priority, bool looping, int seek)
{
	pa_threaded_mainloop_lock(loop);
	const std::uint32_t key = mixer.add(name, residency_priority, compression_priority, 1.0f, 1.0f, looping, seek);
	pa_threaded_mainloop_unlock(loop);

	return key;
}

std::uint32_t SoundEngineLinuxPulseAudio::play(const char *name, win::SoundResidencyPriority residency_priority, float compression_priority, float left, float right, bool looping, int seek)
{
	pa_threaded_mainloop_lock(loop);
	const std::uint32_t key = mixer.add(name, residency_priority, compression_priority, left, right, looping, seek);
	pa_threaded_mainloop_unlock(loop);

	return key;
}

void SoundEngineLinuxPulseAudio::apply_effect(std::uint32_t key, SoundEffect *effect)
{
	pa_threaded_mainloop_lock(loop);
	mixer.apply_effect(key, effect);
	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::remove_effect(std::uint32_t key, SoundEffect *effect)
{
	pa_threaded_mainloop_lock(loop);
	mixer.remove_effect(key, effect);
	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::pause(std::uint32_t key)
{
	pa_threaded_mainloop_lock(loop);
	mixer.pause(key);
	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::resume(std::uint32_t key)
{
	pa_threaded_mainloop_lock(loop);
	mixer.resume(key);
	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::stop(std::uint32_t key)
{
	pa_threaded_mainloop_lock(loop);
	mixer.stop(key);
	pa_threaded_mainloop_unlock(loop);
}

void SoundEngineLinuxPulseAudio::config(std::uint32_t key, float left, float right)
{
	pa_threaded_mainloop_lock(loop);
	mixer.config(key, left, right);
	pa_threaded_mainloop_unlock(loop);
}

}

#endif
