#include <win/win.hpp>

#if defined WINPLAT_LINUX
#include <unistd.h>

#include <win/soundengine.hpp>

static void callback_connect(pa_context*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
}

static void callback_stream(pa_stream*, void *loop)
{
	pa_threaded_mainloop_signal((pa_threaded_mainloop*)loop, 0);
}

namespace win
{

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

}

#endif
