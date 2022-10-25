#ifndef WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_FUNCTIONS_HPP
#define WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_FUNCTIONS_HPP

#include <dlfcn.h>

#include <pulse/pulseaudio.h>

#include <win/win.hpp>
#include <string>

namespace winpa
{

inline pa_threaded_mainloop* (*pa_threaded_mainloop_new)();
inline pa_mainloop_api* (*pa_threaded_mainloop_get_api)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_lock)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_unlock)(pa_threaded_mainloop*);
inline int (*pa_threaded_mainloop_start)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_stop)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_wait)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_free)(pa_threaded_mainloop*);
inline void (*pa_threaded_mainloop_signal)(pa_threaded_mainloop*, int);

inline pa_context* (*pa_context_new)(pa_mainloop_api*, const char*);
inline pa_operation_state_t (*pa_context_set_state_callback)(pa_context*, pa_context_notify_cb_t, void*);
inline pa_context_state_t (*pa_context_get_state)(const pa_context*);
inline void (*pa_context_disconnect)(pa_context*);
inline int (*pa_context_connect)(pa_context*, const char*, pa_context_flags_t, const pa_spawn_api*);
inline pa_operation* (*pa_context_set_sink_input_volume)(pa_context*, uint32_t, const pa_cvolume*, pa_context_success_cb_t, void*);
inline void (*pa_context_unref)(pa_context*);

inline pa_stream* (*pa_stream_new)(const pa_context*, const char*, const pa_sample_spec*, const pa_channel_map*);
inline size_t (*pa_stream_writable_size)(const pa_stream*);
inline void (*pa_stream_set_state_callback)(const pa_stream*, pa_stream_notify_cb_t, void*);
inline int (*pa_stream_connect_playback)(const pa_stream*, const char*, const pa_buffer_attr*, pa_stream_flags_t, const pa_cvolume*, pa_stream*);
inline pa_stream_state_t (*pa_stream_get_state)(const pa_stream*);
inline int (*pa_stream_disconnect)(pa_stream*);
inline uint32_t (*pa_stream_get_index)(const pa_stream*);
inline int (*pa_stream_is_corked)(const pa_stream*);
inline pa_operation* (*pa_stream_cork)(pa_stream*, int, pa_stream_success_cb_t*, void*);
inline pa_operation* (*pa_stream_drain)(pa_stream*, pa_stream_success_cb_t, void*);
inline int (*pa_stream_write)(pa_stream*, const void*, size_t, pa_free_cb_t, int64_t, pa_seek_mode_t);
inline void (*pa_stream_unref)(pa_stream*);

inline pa_cvolume* (*pa_cvolume_init)(pa_cvolume*);
inline pa_cvolume* (*pa_cvolume_set)(pa_cvolume*, unsigned, pa_volume_t);

inline void (*pa_operation_unref)(pa_operation*);
inline pa_operation_state_t (*pa_operation_get_state)(pa_operation*);

inline void* load_pa_fn(const char *name, void *so)
{
    void *fn = dlsym(so, name);

	if (fn == NULL)
		win::bug("PulseAudio: Couldn't load function " + std::string(name));

	return fn;
}

inline void load_pulseaudio_functions(void *so)
{
	pa_threaded_mainloop_new = (decltype(pa_threaded_mainloop_new))load_pa_fn("pa_threaded_mainloop_new", so);
	pa_threaded_mainloop_get_api = (decltype(pa_threaded_mainloop_get_api))load_pa_fn("pa_threaded_mainloop_get_api", so);
	pa_threaded_mainloop_lock = (decltype(pa_threaded_mainloop_lock))load_pa_fn("pa_threaded_mainloop_lock", so);
	pa_threaded_mainloop_unlock = (decltype(pa_threaded_mainloop_unlock))load_pa_fn("pa_threaded_mainloop_unlock", so);
	pa_threaded_mainloop_start = (decltype(pa_threaded_mainloop_start))load_pa_fn("pa_threaded_mainloop_start", so);
	pa_threaded_mainloop_stop = (decltype(pa_threaded_mainloop_stop))load_pa_fn("pa_threaded_mainloop_stop", so);
	pa_threaded_mainloop_wait = (decltype(pa_threaded_mainloop_wait))load_pa_fn("pa_threaded_mainloop_wait", so);
	pa_threaded_mainloop_free = (decltype(pa_threaded_mainloop_free))load_pa_fn("pa_threaded_mainloop_free", so);
	pa_threaded_mainloop_signal = (decltype(pa_threaded_mainloop_signal))load_pa_fn("pa_threaded_mainloop_signal", so);

	pa_context_new = (decltype(pa_context_new))load_pa_fn("pa_context_new", so);
	pa_context_set_state_callback = (decltype(pa_context_set_state_callback))load_pa_fn("pa_context_set_state_callback", so);
	pa_context_get_state = (decltype(pa_context_get_state))load_pa_fn("pa_context_get_state", so);
	pa_context_disconnect = (decltype(pa_context_disconnect))load_pa_fn("pa_context_disconnect", so);
	pa_context_connect = (decltype(pa_context_connect))load_pa_fn("pa_context_connect", so);
	pa_context_set_sink_input_volume = (decltype(pa_context_set_sink_input_volume))load_pa_fn("pa_context_set_sink_input_volume", so);
	pa_context_unref = (decltype(pa_context_unref))load_pa_fn("pa_context_unref", so);

	pa_stream_new = (decltype(pa_stream_new))load_pa_fn("pa_stream_new", so);
	pa_stream_writable_size = (decltype(pa_stream_writable_size))load_pa_fn("pa_stream_writable_size", so);
	pa_stream_set_state_callback = (decltype(pa_stream_set_state_callback))load_pa_fn("pa_stream_set_state_callback", so);
	pa_stream_connect_playback = (decltype(pa_stream_connect_playback))load_pa_fn("pa_stream_connect_playback", so);
	pa_stream_get_state = (decltype(pa_stream_get_state))load_pa_fn("pa_stream_get_state", so);
	pa_stream_disconnect = (decltype(pa_stream_disconnect))load_pa_fn("pa_stream_disconnect", so);
	pa_stream_get_index = (decltype(pa_stream_get_index))load_pa_fn("pa_stream_get_index", so);
	pa_stream_is_corked = (decltype(pa_stream_is_corked))load_pa_fn("pa_stream_is_corked", so);
	pa_stream_cork = (decltype(pa_stream_cork))load_pa_fn("pa_stream_cork", so);
	pa_stream_drain = (decltype(pa_stream_drain))load_pa_fn("pa_stream_drain", so);
	pa_stream_write = (decltype(pa_stream_write))load_pa_fn("pa_stream_write", so);
	pa_stream_unref = (decltype(pa_stream_unref))load_pa_fn("pa_stream_unref", so);

	pa_cvolume_init = (decltype(pa_cvolume_init))load_pa_fn("pa_cvolume_init", so);
	pa_cvolume_set = (decltype(pa_cvolume_set))load_pa_fn("pa_cvolume_set", so);

	pa_operation_unref = (decltype(pa_operation_unref))load_pa_fn("pa_operation_unref", so);
	pa_operation_get_state = (decltype(pa_operation_get_state))load_pa_fn("pa_operation_get_state", so);
}

}

#endif
