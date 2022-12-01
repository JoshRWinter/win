#define WIN_EXTERN_STORAGE
#include <win/sound/soundengine_linux_pulseaudio_functions.hpp>

static void* load_pa_fn(const char *name, void *so)
{
	void *fn = dlsym(so, name);

	if (fn == NULL)
		win::bug("PulseAudio: Couldn't load function " + std::string(name));

	return fn;
}

namespace win
{

void load_pulseaudio_functions(void *so)
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
	pa_context_set_state_callback = (decltype(pa_context_set_state_callback)) load_pa_fn("pa_context_set_state_callback", so);
	pa_context_get_state = (decltype(pa_context_get_state)) load_pa_fn("pa_context_get_state", so);
	pa_context_disconnect = (decltype(pa_context_disconnect)) load_pa_fn("pa_context_disconnect", so);
	pa_context_connect = (decltype(pa_context_connect)) load_pa_fn("pa_context_connect", so);
	pa_context_unref = (decltype(pa_context_unref)) load_pa_fn("pa_context_unref", so);

	pa_stream_new = (decltype(pa_stream_new)) load_pa_fn("pa_stream_new", so);
	pa_stream_writable_size = (decltype(pa_stream_writable_size)) load_pa_fn("pa_stream_writable_size", so);
	pa_stream_set_state_callback = (decltype(pa_stream_set_state_callback)) load_pa_fn("pa_stream_set_state_callback", so);
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
	pa_operation_get_state = (decltype(pa_operation_get_state)) load_pa_fn("pa_operation_get_state", so);
}

}
