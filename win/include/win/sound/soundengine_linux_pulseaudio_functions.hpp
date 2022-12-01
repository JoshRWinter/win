#ifndef WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_FUNCTIONS_HPP
#define WIN_SOUND_ENGINE_LINUX_PULSEAUDIO_FUNCTIONS_HPP

#include <dlfcn.h>

#include <pulse/pulseaudio.h>

#include <win/win.hpp>
#include <string>

#ifdef WIN_EXTERN_STORAGE
#define WIN_EXTERN
#else
#define WIN_EXTERN extern
#endif

namespace win
{

WIN_EXTERN pa_threaded_mainloop* (*pa_threaded_mainloop_new)();
WIN_EXTERN pa_mainloop_api* (*pa_threaded_mainloop_get_api)(pa_threaded_mainloop*);
WIN_EXTERN void (*pa_threaded_mainloop_lock)(pa_threaded_mainloop *loop);
WIN_EXTERN void (*pa_threaded_mainloop_unlock)(pa_threaded_mainloop *loop);
WIN_EXTERN int (*pa_threaded_mainloop_start)(pa_threaded_mainloop *loop);
WIN_EXTERN void (*pa_threaded_mainloop_stop)(pa_threaded_mainloop *loop);
WIN_EXTERN void (*pa_threaded_mainloop_free)(pa_threaded_mainloop *loop);

WIN_EXTERN pa_context* (*pa_context_new)(pa_mainloop_api *loop, const char *name);
WIN_EXTERN pa_operation_state_t (*pa_context_set_state_callback)(pa_context *context, pa_context_notify_cb_t cb, void *userdata);
WIN_EXTERN pa_context_state_t (*pa_context_get_state)(const pa_context *context);
WIN_EXTERN void (*pa_context_disconnect)(pa_context *context);
WIN_EXTERN int (*pa_context_connect)(pa_context *context, const char *server, pa_context_flags_t flags, const pa_spawn_api *api);
WIN_EXTERN void (*pa_context_unref)(pa_context *context);

WIN_EXTERN pa_stream* (*pa_stream_new)(const pa_context *context, const char *name, const pa_sample_spec *ss, const pa_channel_map *map);
WIN_EXTERN size_t (*pa_stream_writable_size)(const pa_stream *stream);
WIN_EXTERN void (*pa_stream_set_state_callback)(const pa_stream *stream, pa_stream_notify_cb_t cb, void *userdata);
WIN_EXTERN int (*pa_stream_connect_playback)(const pa_stream *stream, const char *dev, const pa_buffer_attr *attr, pa_stream_flags_t flags, const pa_cvolume *volume, pa_stream *sync_stream);
WIN_EXTERN void (*pa_stream_set_write_callback)(pa_stream *stream, pa_stream_request_cb_t callback, void *userdata);
WIN_EXTERN void (*pa_stream_set_underflow_callback)(pa_stream *stream, pa_stream_notify_cb_t callback, void *userdata);
WIN_EXTERN int (*pa_stream_begin_write)(pa_stream *stream, void **data, size_t *nbytes);
WIN_EXTERN int (*pa_stream_write)(pa_stream *stream, const void *data, size_t nbytes, pa_free_cb_t free_cb, int64_t offset, pa_seek_mode_t seek);
WIN_EXTERN pa_stream_state_t (*pa_stream_get_state)(const pa_stream *stream);
WIN_EXTERN int (*pa_stream_disconnect)(pa_stream *stream);
WIN_EXTERN void (*pa_stream_unref)(pa_stream *stream);
WIN_EXTERN pa_operation *(*pa_stream_cork)(pa_stream*, int, pa_stream_success_cb_t, void*);

WIN_EXTERN void (*pa_operation_unref)(pa_operation *op);
WIN_EXTERN pa_operation_state_t (*pa_operation_get_state)(pa_operation *op);

void load_pulseaudio_functions(void*);

}

#endif
