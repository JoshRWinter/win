#include <dlfcn.h>

#include  <win/win.hpp>
#define WIN_EXTERN_STORAGE
#include <win/sound/soundengine_linux_pipewire_functions.hpp>

void *load_fn(void *so, const char *name)
{
	void *fn = dlsym(so, name);

	if (fn == NULL)
		win::bug("PipeWire: Couldn't load function " + std::string(name));

	return fn;
}

namespace win
{

void load_pipewire_functions(void *so)
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

	pw_stream_dequeue_buffer = (decltype(pw_stream_dequeue_buffer))load_fn(so, "pw_stream_dequeue_buffer");
	pw_stream_queue_buffer = (decltype(pw_stream_queue_buffer))load_fn(so, "pw_stream_queue_buffer");

	pw_properties_new = (decltype(pw_properties_new))load_fn(so, "pw_properties_new");
}

}
