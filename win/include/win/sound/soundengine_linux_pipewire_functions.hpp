#ifndef WIN_SOUND_ENGINE_LINUX_PIPEWIRE_FUNCTIONS_HPP
#define WIN_SOUND_ENGINE_LINUX_PIPEWIRE_FUNCTIONS_HPP

#include <win/win.hpp>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>

#ifdef WIN_EXTERN_STORAGE
#define WIN_EXTERN
#else
#define WIN_EXTERN extern
#endif

namespace win
{

WIN_EXTERN void (*pw_init)(int *argc, char ***argv);
WIN_EXTERN void (*pw_deinit)();

WIN_EXTERN pw_thread_loop *(*pw_thread_loop_new)(const char *name, const spa_dict *props);
WIN_EXTERN int (*pw_thread_loop_start)(pw_thread_loop *loop);
WIN_EXTERN pw_loop *(*pw_thread_loop_get_loop)(pw_thread_loop *loop);
WIN_EXTERN void (*pw_thread_loop_lock)(pw_thread_loop *loop);
WIN_EXTERN void (*pw_thread_loop_unlock)(pw_thread_loop *loop);
WIN_EXTERN void (*pw_thread_loop_stop)(pw_thread_loop *loop);
WIN_EXTERN void (*pw_thread_loop_destroy)(pw_thread_loop *loop);

WIN_EXTERN pw_stream *(*pw_stream_new_simple)(pw_loop *loop, const char *name, pw_properties *props, pw_stream_events *events, void *data);
WIN_EXTERN int (*pw_stream_connect)(pw_stream *stream, spa_direction direction, uint32_t target_id, pw_stream_flags flags, const spa_pod **params, uint32_t nparams);
WIN_EXTERN pw_stream_state (*pw_stream_get_state)(pw_stream *stream, const char **error);
WIN_EXTERN int (*pw_stream_disconnect)(pw_stream *stream);
WIN_EXTERN void (*pw_stream_destroy)(pw_stream *stream);

WIN_EXTERN pw_buffer *(*pw_stream_dequeue_buffer)(pw_stream *stream);
WIN_EXTERN int (*pw_stream_queue_buffer)(pw_stream *stream, pw_buffer *buffer);

WIN_EXTERN pw_properties *(*pw_properties_new)(const char *key, ...);

void load_pipewire_functions(void *so);

}

#endif
