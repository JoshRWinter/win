#pragma once

#include <memory>

#include <win/DisplayBase.hpp>

#if defined WINPLAT_WINDOWS
#include <win/Win32Display.hpp>
#elif defined WINPLAT_LINUX
#include <win/WaylandDisplay.hpp>
#endif

namespace win
{

class Display
{
    WIN_NO_COPY_MOVE(Display);

public:
    explicit Display(const DisplayOptions &options)
    {
#if defined WINPLAT_WINDOWS
        inner.reset(new Win32Display(options));
#elif defined WINPLAT_LINUX
        inner.reset(new WaylandDisplay(options));
#endif
    }

    void process() { inner->process(); }

    void swap() { inner->swap(); }

    int width() { return inner->width(); }

    int height() { return inner->height(); }

    void resize(int w, int h) { inner->resize(w, h); }

    float refresh_rate() { return inner->refresh_rate(); }

    void show_pointer(bool show) { inner->show_pointer(show); }

    void lock_pointer(bool lock) { inner->lock_pointer(lock); }

    void set_fullscreen(bool fullscreen) { inner->set_fullscreen(fullscreen); }

    void vsync(bool on) { inner->vsync(on); }

    NativeWindowHandle native_handle() { return inner->native_handle(); }

    void register_window_handler(DisplayBase::WindowHandler handler) { inner->register_window_handler(std::move(handler)); }

    void register_resize_handler(DisplayBase::ResizeHandler handler) { inner->register_resize_handler(std::move(handler)); }

    void register_button_handler(DisplayBase::ButtonHandler handler) { inner->register_button_handler(std::move(handler)); }

    void register_character_handler(DisplayBase::CharacterHandler handler) { inner->register_character_handler(std::move(handler)); }

    void register_mouse_handler(DisplayBase::MouseHandler handler) { inner->register_mouse_handler(std::move(handler)); }

    void register_relative_mouse_handler(DisplayBase::RelativeMouseHandler handler) { inner->register_relative_mouse_handler(std::move(handler)); }

private:
    std::unique_ptr<DisplayBase> inner;
};

}
