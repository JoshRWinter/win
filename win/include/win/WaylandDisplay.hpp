#pragma once

#include <array>
#include <map>

#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

#include <win/DisplayBase.hpp>

#include "WaylandPointerConstraints.h"
#include "WaylandRelativePointer.h"
#include "WaylandTearingControl.h"
#include "WaylandXdg.h"

namespace win
{

class WaylandDisplay : public win::DisplayBase
{
    WIN_NO_COPY_MOVE(WaylandDisplay);

    struct Output
    {
        int width = 0, height = 0;
        float refresh = 60.0f;
    };

public:
    explicit WaylandDisplay(const win::DisplayOptions &options);
    ~WaylandDisplay() override;

    void process() override;
    void swap() override;
    int width() override;
    int height() override;
    void resize(int w, int h) override;
    float refresh_rate() override;
    void show_pointer(bool show) override;
    void lock_pointer(bool lock) override;
    void set_fullscreen(bool fullscreen) override;
    void vsync(bool) override;
    win::NativeWindowHandle native_handle() override;

private:
    void load_normal_pointer(wl_pointer *pointer, uint32_t serial);

    static void registry_add_object(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void registry_remove_object(void *data, wl_registry *registry, uint32_t name);

    static void wl_seat_listener_capabilities(void *data, wl_seat *seat, uint32_t capabilities);
    static void wl_pointer_listener_enter(void *data, wl_pointer *seat, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y);
    static void wl_pointer_listener_motion(void *data, wl_pointer *seat, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    static void wl_pointer_listener_button(void *data, wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    static void zwp_relative_pointer_listener_relative_motion(void *data,
                                                              zwp_relative_pointer_v1 *pointer,
                                                              uint32_t lo,
                                                              uint32_t hi,
                                                              wl_fixed_t dx,
                                                              wl_fixed_t dy,
                                                              wl_fixed_t dx_unaccel,
                                                              wl_fixed_t dy_unaccel);
    static void wl_keyboard_listener_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void wl_keyboard_listener_keymap(void *data, wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size);
    static void wl_keyboard_listener_modifiers(void *data,
                                               wl_keyboard *keyboard,
                                               uint32_t serial,
                                               uint32_t mods_depressed,
                                               uint32_t mods_latched,
                                               uint32_t mods_locked,
                                               uint32_t group);
    static void wl_output_listener_mode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
    static void wl_surface_listener_enter(void *data, wl_surface *surface, wl_output *output);
    static void wl_surface_listener_leave(void *data, wl_surface *surface, wl_output *output);
    static void xdg_wm_base_listener_pong(void *data, xdg_wm_base *wm_base, uint32_t serial);
    static void xdg_surface_listener_configure(void *data, xdg_surface *surface, uint32_t serial);
    static void xdg_toplevel_listener_configure(void *data, xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states);
    static void xdg_toplevel_listener_close(void *data, xdg_toplevel *toplevel);

    static constexpr int button_map_size = 112;
    std::array<win::Button, button_map_size> button_map;
    void init_button_map();

    struct
    {
        int width = 0, height = 0;
    	int relx = 0, rely = 0;
        float refresh = 60.0f;
        bool resized = false;
        std::chrono::time_point<std::chrono::steady_clock> resize_time;
        bool cursor_locked = false;
        bool cursor_hidden = false;
        bool go_fullscreen = false;
    } props;

    struct
    {
        wl_display *display = NULL;
        wl_registry_listener registry_listener { .global = registry_add_object, .global_remove = registry_remove_object };

        wl_seat *seat = NULL;
        wl_seat_listener seat_listener { .capabilities = wl_seat_listener_capabilities };

        wl_pointer *pointer = NULL;
        uint32_t latest_enter_serial = 0;
        wl_pointer_listener pointer_listener { .enter = wl_pointer_listener_enter,
                                               .leave = [](void *, wl_pointer *, uint32_t, wl_surface *) {},
                                               .motion = wl_pointer_listener_motion,
                                               .button = wl_pointer_listener_button,
                                               .axis = [](void *, wl_pointer *, uint32_t, uint32_t, wl_fixed_t) {} };

        zwp_relative_pointer_manager_v1 *relative_pointer_manager = NULL;
        zwp_relative_pointer_v1 *relative_pointer = NULL;
        zwp_relative_pointer_v1_listener relative_pointer_listener { .relative_motion = zwp_relative_pointer_listener_relative_motion };

        zwp_pointer_constraints_v1 *pointer_constraints = NULL;
        zwp_confined_pointer_v1 *confined_pointer = NULL;

        xkb_context *xkb = NULL;
        xkb_keymap *xkbkeymap = NULL;
        xkb_state *xkbstate = NULL;
        wl_keyboard *keyboard = NULL;
        wl_keyboard_listener keyboard_listener = { .keymap = wl_keyboard_listener_keymap,
                                                   .enter = [](void *, wl_keyboard *, uint32_t, wl_surface *, wl_array *) {},
                                                   .leave = [](void *, wl_keyboard *, uint32_t, wl_surface *) {},
                                                   .key = wl_keyboard_listener_key,
                                                   .modifiers = wl_keyboard_listener_modifiers };

        wl_shm *shm = NULL;

        std::map<wl_output *, Output> outputs;
        wl_output *current_output = NULL;
        int current_outputs = 0;
        wl_output_listener output_listener = { .geometry =
                                                   [](void *, wl_output *, int32_t, int32_t, int32_t, int32_t, int32_t, const char *, const char *, int32_t) {},
                                               .mode = wl_output_listener_mode,
                                               .done = [](void *, wl_output *) {},
                                               .scale = [](void *, wl_output *, int32_t) {},
                                               .name = [](void *, wl_output *, const char *name) {},
                                               .description = [](void *, wl_output *, const char *) {} };

        wp_tearing_control_manager_v1 *tearing_control_manager = NULL;
        wp_tearing_control_v1 *tearing_control = NULL;

        wl_cursor_theme *cursor_theme = NULL;
        wl_cursor *cursor = NULL;
        wl_surface *cursor_surface = NULL;
        wl_surface_listener surface_listener { .enter = wl_surface_listener_enter,
                                               .leave = wl_surface_listener_leave }; //[](void *, wl_surface *, wl_output *) {} };

        wl_registry *registry = NULL;
        wl_compositor *compositor = NULL;
        wl_surface *surface = NULL;
        wl_egl_window *egl_window = NULL;
    } wl;

    struct
    {
        xdg_wm_base *wm_base = NULL;
        xdg_wm_base_listener wm_base_listener { .ping = xdg_wm_base_listener_pong };
        xdg_surface *surface = NULL;
        xdg_surface_listener surface_listener { .configure = xdg_surface_listener_configure };
        xdg_toplevel *toplevel = NULL;
        xdg_toplevel_listener toplevel_listener { .configure = xdg_toplevel_listener_configure, .close = xdg_toplevel_listener_close };
    } xdg;

    struct
    {
        EGLDisplay display;
        EGLContext context;
        EGLSurface surface;
    } egl;
};

}
