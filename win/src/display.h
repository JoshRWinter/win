#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

#include <functional>
#include <memory>

namespace win
{

struct display_remote;
class display
{
	friend class system;
	friend class audio_engine;
	friend struct display_remote;

	typedef std::function<void(button, bool)> fn_event_button;
	typedef std::function<void(joystick_axis, int)> fn_event_joystick;
	typedef std::function<void(int)> fn_event_character;
	typedef std::function<void(int x, int y)> fn_event_mouse;

public:
	static constexpr int FULLSCREEN = 1;

	display() = default;
	display(const display&) = delete;
	display(display&&);
	~display();

	display &operator=(display&&);

	bool process();
	void swap() const;
	int width() const;
	int height() const;
	void cursor(bool);
	void vsync(bool);

	void event_button(fn_event_button);
	void event_joystick(fn_event_joystick);
	void event_character(fn_event_character);
	void event_mouse(fn_event_mouse);

	audio_engine make_audio_engine(audio_engine::sound_config_fn);
	font_renderer make_font_renderer(int, int, float, float, float, float) const;

	static int screen_width();
	static int screen_height();

private:
	display(const char*, int, int, int, window_handle);

#if defined WINPLAT_WINDOWS
	static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	static void win_init_gl(display_remote*, HWND);
	void win_term_gl();
#endif

	std::unique_ptr<display_remote> remote;
	void process_joystick();
	void finalize();
};

struct display_remote
{
	display_remote() = default;
	display_remote(const display_remote&) = delete;
	display_remote(display_remote&&) = delete;
	void operator=(const display_remote&) = delete;
	void operator=(display_remote&&) = delete;
	struct
	{
		// keyboard
		win::display::fn_event_button key_button;
		win::display::fn_event_character character;

		// mouse
		win::display::fn_event_mouse mouse;
	}handler;

#if defined WINPLAT_LINUX
	Window window_;
	GLXContext context_;
	evdev_joystick joystick_;
#elif defined WINPLAT_WINDOWS
	HWND window_;
	HDC hdc_;
	HGLRC context_;
	win::audio_engine_remote *directsound_; // non-owning
	bool winquit_;
#endif
};

}

#endif
