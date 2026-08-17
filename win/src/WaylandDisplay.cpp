#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <cstring>

#include <linux/input.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include <win/WaylandDisplay.hpp>

win::WaylandDisplay::WaylandDisplay(const win::DisplayOptions &options)
{
    init_button_map();

    props.go_fullscreen = options.fullscreen;

    wl.display = wl_display_connect(NULL);
    if (wl.display == NULL)
        win::bug("WaylandDisplay: Couldn't connect to wayland display");

    wl.registry = wl_display_get_registry(wl.display);
    wl_registry_add_listener(wl.registry, &wl.registry_listener, this);

    wl_display_roundtrip(wl.display);

    if (wl.compositor == NULL)
        win::bug("WaylandDisplay: Couldn't get wayland compositor");

    wl.cursor_surface = wl_compositor_create_surface(wl.compositor);

    wl.surface = wl_compositor_create_surface(wl.compositor);
    if (wl.surface == NULL)
        win::bug("WaylandDisplay: Couldn't create wayland surface");
    wl_surface_add_listener(wl.surface, &wl.surface_listener, this);

    wl.tearing_control = wp_tearing_control_manager_v1_get_tearing_control(wl.tearing_control_manager, wl.surface);

    if (xdg.wm_base == NULL)
        win::bug("WaylandDisplay: couldn't create xdg base");

    xdg.surface = xdg_wm_base_get_xdg_surface(xdg.wm_base, wl.surface);
    xdg_surface_add_listener(xdg.surface, &xdg.surface_listener, this);

    xdg.toplevel = xdg_surface_get_toplevel(xdg.surface);
    if (!xdg.toplevel)
        win::bug("WaylandDisplay: couldn't get xdg toplevel");

    xdg_toplevel_add_listener(xdg.toplevel, &xdg.toplevel_listener, this);
    xdg_toplevel_set_title(xdg.toplevel, options.caption.c_str());

    wl_surface_commit(wl.surface);

    wl.egl_window = wl_egl_window_create(wl.surface, options.width, options.height);
    if (wl.egl_window == NULL)
        win::bug("WaylandDisplay: couldn't create egl window");

    egl.display = eglGetDisplay(wl.display);
    if (egl.display == EGL_NO_DISPLAY)
        win::bug("WaylandDisplay: Couldn't get egl display");

    if (!eglInitialize(egl.display, NULL, NULL))
        win::bug("WaylandDisplay: Couldn't initialize egl");

    // clang-format off
    const EGLint attrs[]
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_CONFORMANT, EGL_OPENGL_BIT,
        EGL_NONE
    };
    // clang-format on

    EGLint configs;
    EGLConfig config;
    if (eglChooseConfig(egl.display, attrs, &config, 1, &configs) == EGL_FALSE || configs == 0)
        win::bug("WaylandDisplay: Couldn't load egl configs");

    if (!eglBindAPI(EGL_OPENGL_API))
        win::bug("WaylandDisplay: Couldn't bind opengl");

    // clang-format off
    const EGLint context_attrs[]
    {
        EGL_CONTEXT_MAJOR_VERSION, options.gl_major,
        EGL_CONTEXT_MINOR_VERSION, options.gl_minor,
        EGL_CONTEXT_OPENGL_DEBUG, options.debug ? EGL_TRUE : EGL_FALSE,
        EGL_NONE
    };
    // clang-format on

    egl.context = eglCreateContext(egl.display, config, EGL_NO_CONTEXT, context_attrs);
    if (egl.context == EGL_NO_CONTEXT)
        win::bug("WaylandDisplay: Couldn't create egl context");

    egl.surface = eglCreateWindowSurface(egl.display, config, (EGLNativeWindowType)wl.egl_window, NULL);

    if (!eglMakeCurrent(egl.display, egl.surface, egl.surface, egl.context))
        win::bug("WaylandDisplay: Couldn't make egl context current");

    props.width = options.width;
    props.height = options.height;

    wl.xkb = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wl_display_dispatch_pending(wl.display);
}

win::WaylandDisplay::~WaylandDisplay()
{
    xkb_context_unref(wl.xkb);

    if (wl.xkbkeymap != NULL)
        xkb_keymap_unref(wl.xkbkeymap);

    if (wl.xkbstate != NULL)
        xkb_state_unref(wl.xkbstate);

    eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroySurface(egl.display, egl.surface);
    eglDestroyContext(egl.display, egl.context);
    eglTerminate(egl.display);
    wl_egl_window_destroy(wl.egl_window);

    if (wl.relative_pointer_manager != NULL)
        zwp_relative_pointer_manager_v1_destroy(wl.relative_pointer_manager);

    if (wl.relative_pointer != NULL)
        zwp_relative_pointer_v1_destroy(wl.relative_pointer);

    if (wl.pointer_constraints != NULL)
        zwp_pointer_constraints_v1_destroy(wl.pointer_constraints);

    if (wl.confined_pointer != NULL)
        zwp_confined_pointer_v1_destroy(wl.confined_pointer);

    if (wl.pointer != NULL)
        wl_pointer_destroy(wl.pointer);

    if (wl.seat != NULL)
        wl_seat_destroy(wl.seat);

    if (wl.cursor_theme != NULL)
        wl_cursor_theme_destroy(wl.cursor_theme);

    if (wl.keyboard != NULL)
        wl_keyboard_destroy(wl.keyboard);

    if (wl.shm != NULL)
        wl_shm_destroy(wl.shm);

    for (auto output : wl.outputs)
        wl_output_destroy(output.first);

    if (wl.tearing_control != NULL)
        wp_tearing_control_v1_destroy(wl.tearing_control);

    if (wl.tearing_control_manager != NULL)
        wp_tearing_control_manager_v1_destroy(wl.tearing_control_manager);

    xdg_toplevel_destroy(xdg.toplevel);
    xdg_surface_destroy(xdg.surface);
    xdg_wm_base_destroy(xdg.wm_base);
    wl_surface_destroy(wl.surface);
    wl_surface_destroy(wl.cursor_surface);
    wl_compositor_destroy(wl.compositor);
    wl_registry_destroy(wl.registry);
    wl_display_disconnect(wl.display);
}

void win::WaylandDisplay::process()
{
    wl_display_dispatch_pending(wl.display);

	if (props.relx != 0 || props.rely != 0)
		relative_mouse_handler(props.relx, props.rely);

	props.relx = 0;
	props.rely = 0;

    if (props.resized && std::chrono::duration<float>(std::chrono::steady_clock::now() - props.resize_time).count() > 0.8f)
    {
        props.resized = false;
        resize_handler(props.width, props.height);
    }
}

void win::WaylandDisplay::swap()
{
    eglSwapBuffers(egl.display, egl.surface);
}

int win::WaylandDisplay::width()
{
    return props.width;
}

int win::WaylandDisplay::height()
{
    return props.height;
}

void win::WaylandDisplay::resize(int w, int h)
{
    fprintf(stderr, "WaylandDisplay: resize not supported\n");
}

float win::WaylandDisplay::refresh_rate()
{
    return props.refresh;
}

void win::WaylandDisplay::show_pointer(bool show)
{
    props.cursor_hidden = !show;

    if (wl.latest_enter_serial != 0)
    {
        if (show)
            load_normal_pointer(wl.pointer, wl.latest_enter_serial);
        else
            wl_pointer_set_cursor(wl.pointer, wl.latest_enter_serial, NULL, 0, 0);
    }
}

void win::WaylandDisplay::lock_pointer(bool lock)
{
    if (lock)
    {
        if (wl.pointer_constraints == NULL)
            fprintf(stderr, "WaylandDisplay: confining the pointer is not supported on this system\n");
        else
        {
            if (wl.confined_pointer == NULL)
                wl.confined_pointer = zwp_pointer_constraints_v1_confine_pointer(wl.pointer_constraints,
                                                                                 wl.surface,
                                                                                 wl.pointer,
                                                                                 NULL,
                                                                                 ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
        }
    }
    else
    {
        if (wl.confined_pointer != NULL)
        {
            zwp_confined_pointer_v1_destroy(wl.confined_pointer);
            wl.confined_pointer = NULL;
        }
    }
}

void win::WaylandDisplay::set_fullscreen(bool fullscreen)
{
    if (fullscreen)
        xdg_toplevel_set_fullscreen(xdg.toplevel, wl.current_output);
    else
        xdg_toplevel_unset_fullscreen(xdg.toplevel);
}

void win::WaylandDisplay::vsync(bool on)
{
    if (wl.tearing_control_manager == NULL)
        fprintf(stderr, "WaylandDisplay: missing support for wp_tearing_control_manager_v1\n");
    else if (on)
        wp_tearing_control_v1_set_presentation_hint(wl.tearing_control, WP_TEARING_CONTROL_V1_PRESENTATION_HINT_VSYNC);
    else
        wp_tearing_control_v1_set_presentation_hint(wl.tearing_control, WP_TEARING_CONTROL_V1_PRESENTATION_HINT_ASYNC);
}

win::NativeWindowHandle win::WaylandDisplay::native_handle()
{
    return NULL;
}

void win::WaylandDisplay::load_normal_pointer(wl_pointer *pointer, uint32_t serial)
{
    auto image = wl.cursor->images[0];
    auto buffer = wl_cursor_image_get_buffer(image);
    wl_pointer_set_cursor(pointer, serial, wl.cursor_surface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(wl.cursor_surface, buffer, 0, 0);
    wl_surface_damage(wl.cursor_surface, 0, 0, image->width, image->height);
    wl_surface_commit(wl.cursor_surface);
}

void win::WaylandDisplay::registry_add_object(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto &wd = *(WaylandDisplay *)data;

    if (!strcmp(interface, wl_compositor_interface.name))
    {
        wd.wl.compositor = (wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (!strcmp(interface, xdg_wm_base_interface.name))
    {
        wd.xdg.wm_base = (xdg_wm_base *)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(wd.xdg.wm_base, &wd.xdg.wm_base_listener, data);
    }
    else if (!strcmp(interface, wl_seat_interface.name))
    {
        wd.wl.seat = (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(wd.wl.seat, &wd.wl.seat_listener, data);
    }
    else if (!strcmp(interface, zwp_relative_pointer_manager_v1_interface.name))
    {
        wd.wl.relative_pointer_manager = (zwp_relative_pointer_manager_v1 *)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
    }
    else if (!strcmp(interface, zwp_pointer_constraints_v1_interface.name))
    {
        wd.wl.pointer_constraints = (zwp_pointer_constraints_v1 *)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
    }
    else if (!strcmp(interface, wl_shm_interface.name))
    {
        wd.wl.shm = (wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
        wd.wl.cursor_theme = wl_cursor_theme_load(NULL, 16, wd.wl.shm);
        wd.wl.cursor = wl_cursor_theme_get_cursor(wd.wl.cursor_theme, "left_ptr");
    }
    else if (!strcmp(interface, wl_output_interface.name))
    {
        auto output = (wl_output *)wl_registry_bind(registry, name, &wl_output_interface, 4);
        wd.wl.outputs.emplace(output, Output());
        wl_output_add_listener(output, &wd.wl.output_listener, data);
    }
    else if (!strcmp(interface, wp_tearing_control_manager_v1_interface.name))
    {
        wd.wl.tearing_control_manager = (wp_tearing_control_manager_v1 *)wl_registry_bind(registry, name, &wp_tearing_control_manager_v1_interface, 1);
    }
}

void win::WaylandDisplay::registry_remove_object(void *data, wl_registry *registry, uint32_t name) {}

void win::WaylandDisplay::wl_seat_listener_capabilities(void *data, wl_seat *seat, uint32_t capabilities)
{
    auto &wd = *(WaylandDisplay *)data;

    if (capabilities & WL_SEAT_CAPABILITY_POINTER)
    {
        wd.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(wd.wl.pointer, &wd.wl.pointer_listener, data);
        if (wd.wl.relative_pointer_manager != NULL)
        {
            wd.wl.relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(wd.wl.relative_pointer_manager, wd.wl.pointer);
            zwp_relative_pointer_v1_add_listener(wd.wl.relative_pointer, &wd.wl.relative_pointer_listener, data);
        }
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        wd.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(wd.wl.keyboard, &wd.wl.keyboard_listener, data);
    }
}

void win::WaylandDisplay::wl_pointer_listener_enter(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    auto &wd = *(WaylandDisplay *)data;
    wd.wl.latest_enter_serial = serial;

    if (wd.props.cursor_hidden)
        wl_pointer_set_cursor(wd.wl.pointer, wd.wl.latest_enter_serial, NULL, 0, 0);
    else
        wd.load_normal_pointer(pointer, serial);
}

void win::WaylandDisplay::wl_pointer_listener_motion(void *data, wl_pointer *seat, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    auto &wd = *(WaylandDisplay *)data;
    wd.mouse_handler(wl_fixed_to_int(x), wl_fixed_to_int(y));
}

void win::WaylandDisplay::wl_pointer_listener_button(void *data, wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    auto &wd = *(WaylandDisplay *)data;
    switch (button)
    {
        case BTN_LEFT:
            wd.button_handler(win::Button::mouse_left, state == 1);
            break;
        case BTN_RIGHT:
            wd.button_handler(win::Button::mouse_right, state == 1);
            break;
        case BTN_MIDDLE:
            wd.button_handler(win::Button::mouse_middle, state == 1);
            break;
        default:
            break;
    }
}

void win::WaylandDisplay::zwp_relative_pointer_listener_relative_motion(void *data,
                                                                        zwp_relative_pointer_v1 *pointer,
                                                                        uint32_t lo,
                                                                        uint32_t hi,
                                                                        wl_fixed_t dx,
                                                                        wl_fixed_t dy,
                                                                        wl_fixed_t dx_unaccel,
                                                                        wl_fixed_t dy_unaccel)
{
    auto &wd = *(WaylandDisplay *)data;

	wd.props.relx += wl_fixed_to_int(dx_unaccel);
	wd.props.rely += wl_fixed_to_int(dy_unaccel);
}

void win::WaylandDisplay::wl_keyboard_listener_keymap(void *data, wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    auto &wd = *(WaylandDisplay *)data;
    if (keyboard == wd.wl.keyboard && format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        if (wd.wl.xkbkeymap != NULL)
            xkb_keymap_unref(wd.wl.xkbkeymap);
        if (wd.wl.xkbstate != NULL)
            xkb_state_unref(wd.wl.xkbstate);

        auto map = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (map == MAP_FAILED)
        {
            fprintf(stderr, "WaylandDisplay: Couldn't map keymap fd\n");
            close(fd);
            return;
        }

        wd.wl.xkbkeymap = xkb_keymap_new_from_string(wd.wl.xkb, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
        wd.wl.xkbstate = xkb_state_new(wd.wl.xkbkeymap);

        munmap(map, size);
        close(fd);
    }
}

void win::WaylandDisplay::wl_keyboard_listener_key(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    const auto keycode = key + 8;
    auto &wd = *(WaylandDisplay *)data;

    if (key < button_map_size && key > 0)
    {
        auto b = wd.button_map[key];
        if (b != win::Button::undefined)
            wd.button_handler(b, state == 1);
    }

    if (state)
    {
        auto sym = xkb_state_key_get_one_sym(wd.wl.xkbstate, keycode);

        if (sym >= ' ' && sym <= '~')
            wd.character_handler((char)sym);
        else if (sym == XKB_KEY_BackSpace)
            wd.character_handler('\b');
        else if (sym == XKB_KEY_Return)
            wd.character_handler('\n');
    }
}

void win::WaylandDisplay::wl_keyboard_listener_modifiers(void *data,
                                                         wl_keyboard *keyboard,
                                                         uint32_t serial,
                                                         uint32_t mods_depressed,
                                                         uint32_t mods_latched,
                                                         uint32_t mods_locked,
                                                         uint32_t group)
{
    auto &wd = *(WaylandDisplay *)data;
    xkb_state_update_mask(wd.wl.xkbstate, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void win::WaylandDisplay::wl_output_listener_mode(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    auto &wd = *(WaylandDisplay *)data;
    wd.wl.outputs.at(output).refresh = refresh / 1000.0f;
}

void win::WaylandDisplay::wl_surface_listener_enter(void *data, wl_surface *surface, wl_output *output)
{
    auto &wd = *(WaylandDisplay *)data;

    if (wd.wl.current_outputs == 0)
        wd.props.refresh = wd.wl.outputs.contains(output) ? wd.wl.outputs.at(output).refresh : 60.0f;

    wd.wl.current_output = output;

    ++wd.wl.current_outputs;

    if (wd.props.go_fullscreen)
    {
        xdg_toplevel_set_fullscreen(wd.xdg.toplevel, output);
        wd.props.go_fullscreen = false;
    }
}

void win::WaylandDisplay::wl_surface_listener_leave(void *data, wl_surface *surface, wl_output *output)
{
    auto &wd = *(WaylandDisplay *)data;

    if (--wd.wl.current_outputs == 1)
        wd.props.refresh = wd.wl.outputs.contains(wd.wl.current_output) ? wd.wl.outputs.at(wd.wl.current_output).refresh : 60.0f;
}

void win::WaylandDisplay::xdg_wm_base_listener_pong(void *data, xdg_wm_base *wm_base, uint32_t serial)
{
    xdg_wm_base_pong(wm_base, serial);
}

void win::WaylandDisplay::xdg_surface_listener_configure(void *data, xdg_surface *surface, uint32_t serial)
{
    const auto &w = *(win::WaylandDisplay *)data;
    if (w.props.resized)
        wl_egl_window_resize(w.wl.egl_window, w.props.width, w.props.height, 0, 0);
    xdg_surface_ack_configure(surface, serial);
}

void win::WaylandDisplay::xdg_toplevel_listener_configure(void *data, xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states)
{
    auto &w = *(WaylandDisplay *)data;

    bool resize = false;
    if (width != 0 && width != w.props.width)
    {
        w.props.width = width;
        resize = true;
    }
    if (height != 0 && height != w.props.height)
    {
        w.props.height = height;
        resize = true;
    }

    if (resize)
    {
        w.props.resized = true;
        w.props.resize_time = std::chrono::steady_clock::now();
    }
}

void win::WaylandDisplay::xdg_toplevel_listener_close(void *data, xdg_toplevel *toplevel)
{
    ((WaylandDisplay *)data)->window_handler(WindowEvent::close);
}

void win::WaylandDisplay::init_button_map()
{
    for (auto &x : button_map)
        x = win::Button::undefined;

    button_map.at(KEY_ESC) = win::Button::esc;

    button_map.at(KEY_1) = win::Button::d1;
    button_map.at(KEY_2) = win::Button::d2;
    button_map.at(KEY_3) = win::Button::d3;
    button_map.at(KEY_4) = win::Button::d4;
    button_map.at(KEY_5) = win::Button::d5;
    button_map.at(KEY_6) = win::Button::d6;
    button_map.at(KEY_7) = win::Button::d7;
    button_map.at(KEY_8) = win::Button::d8;
    button_map.at(KEY_9) = win::Button::d9;
    button_map.at(KEY_0) = win::Button::d0;

    button_map.at(KEY_MINUS) = win::Button::dash;
    button_map.at(KEY_EQUAL) = win::Button::equal;
    button_map.at(KEY_BACKSPACE) = win::Button::backspace;
    button_map.at(KEY_TAB) = win::Button::tab;

    button_map.at(KEY_LEFTBRACE) = win::Button::lbracket;
    button_map.at(KEY_RIGHTBRACE) = win::Button::rbracket;
    button_map.at(KEY_ENTER) = win::Button::enter;
    button_map.at(KEY_LEFTCTRL) = win::Button::lctrl;
    button_map.at(KEY_RIGHTCTRL) = win::Button::rctrl;
    button_map.at(KEY_SEMICOLON) = win::Button::semicolon;
    button_map.at(KEY_APOSTROPHE) = win::Button::apostrophe;
    button_map.at(KEY_GRAVE) = win::Button::backtick;
    button_map.at(KEY_LEFTSHIFT) = win::Button::lshift;
    button_map.at(KEY_RIGHTSHIFT) = win::Button::rshift;
    button_map.at(KEY_BACKSLASH) = win::Button::backslash;
    button_map.at(KEY_COMMA) = win::Button::comma;
    button_map.at(KEY_DOT) = win::Button::period;
    button_map.at(KEY_SLASH) = win::Button::slash;
    button_map.at(KEY_LEFTALT) = win::Button::lalt;
    button_map.at(KEY_RIGHTALT) = win::Button::ralt;
    button_map.at(KEY_SPACE) = win::Button::space;
    button_map.at(KEY_CAPSLOCK) = win::Button::capslock;
    button_map.at(KEY_NUMLOCK) = win::Button::numlock;
    button_map.at(KEY_HOME) = win::Button::home;
    button_map.at(KEY_PAGEUP) = win::Button::page_up;
    button_map.at(KEY_PAGEDOWN) = win::Button::page_down;
    button_map.at(KEY_DELETE) = win::Button::del;
    button_map.at(KEY_LEFT) = win::Button::left;
    button_map.at(KEY_RIGHT) = win::Button::right;
    button_map.at(KEY_DOWN) = win::Button::down;
    button_map.at(KEY_UP) = win::Button::up;

    button_map.at(KEY_KP1) = win::Button::num1;
    button_map.at(KEY_KP2) = win::Button::num2;
    button_map.at(KEY_KP3) = win::Button::num3;
    button_map.at(KEY_KP4) = win::Button::num4;
    button_map.at(KEY_KP5) = win::Button::num5;
    button_map.at(KEY_KP6) = win::Button::num6;
    button_map.at(KEY_KP7) = win::Button::num7;
    button_map.at(KEY_KP8) = win::Button::num8;
    button_map.at(KEY_KP9) = win::Button::num9;
    button_map.at(KEY_KP0) = win::Button::num0;
    button_map.at(KEY_KPASTERISK) = win::Button::num_star;
    button_map.at(KEY_KPMINUS) = win::Button::num_minus;
    button_map.at(KEY_KPPLUS) = win::Button::num_plus;
    button_map.at(KEY_KPSLASH) = win::Button::num_slash;
    button_map.at(KEY_KPDOT) = win::Button::num_del;
    button_map.at(KEY_KPENTER) = win::Button::enter;

    button_map.at(KEY_F1) = win::Button::f1;
    button_map.at(KEY_F2) = win::Button::f2;
    button_map.at(KEY_F3) = win::Button::f3;
    button_map.at(KEY_F4) = win::Button::f4;
    button_map.at(KEY_F5) = win::Button::f5;
    button_map.at(KEY_F6) = win::Button::f6;
    button_map.at(KEY_F7) = win::Button::f7;
    button_map.at(KEY_F8) = win::Button::f8;
    button_map.at(KEY_F9) = win::Button::f9;
    button_map.at(KEY_F10) = win::Button::f10;
    button_map.at(KEY_F11) = win::Button::f11;
    button_map.at(KEY_F12) = win::Button::f12;

    button_map.at(KEY_Q) = win::Button::q;
    button_map.at(KEY_W) = win::Button::w;
    button_map.at(KEY_E) = win::Button::e;
    button_map.at(KEY_R) = win::Button::r;
    button_map.at(KEY_T) = win::Button::t;
    button_map.at(KEY_Y) = win::Button::y;
    button_map.at(KEY_U) = win::Button::u;
    button_map.at(KEY_I) = win::Button::i;
    button_map.at(KEY_O) = win::Button::o;
    button_map.at(KEY_P) = win::Button::p;
    button_map.at(KEY_A) = win::Button::a;
    button_map.at(KEY_S) = win::Button::s;
    button_map.at(KEY_D) = win::Button::d;
    button_map.at(KEY_F) = win::Button::f;
    button_map.at(KEY_G) = win::Button::g;
    button_map.at(KEY_H) = win::Button::h;
    button_map.at(KEY_J) = win::Button::j;
    button_map.at(KEY_K) = win::Button::k;
    button_map.at(KEY_L) = win::Button::l;
    button_map.at(KEY_Z) = win::Button::z;
    button_map.at(KEY_X) = win::Button::x;
    button_map.at(KEY_C) = win::Button::c;
    button_map.at(KEY_V) = win::Button::v;
    button_map.at(KEY_B) = win::Button::b;
    button_map.at(KEY_N) = win::Button::n;
    button_map.at(KEY_M) = win::Button::m;
}

#endif
