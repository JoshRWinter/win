#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

#include <functional>
#include <memory>

namespace win
{

#ifdef _WIN32
class display;
class indirect
{
	friend class display;
	indirect(display *d) : dsp(d) {}
	win::display *dsp;
};
#endif

class display
{
	friend class system;
	friend class audio_engine;

	typedef std::function<void(button, bool)> fn_event_button;
	typedef std::function<void(joystick_axis, int)> fn_event_joystick;
	typedef std::function<void(int)> fn_event_character;
	typedef std::function<void(int x, int y)> fn_event_mouse;

public:
	static constexpr int FULLSCREEN = 1;

	display();
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
	void process_joystick();
	void move(display&);
	void finalize();

	struct
	{
		// keyboard
		fn_event_button key_button;
		fn_event_character character;

		// mouse
		fn_event_mouse mouse;
	}handler;

#if defined WINPLAT_LINUX
	Window window_;
	GLXContext context_;
	evdev_joystick joystick_;
#elif defined WINPLAT_WINDOWS
	static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	void win_init_gl(HWND);
	void win_term_gl();

	std::unique_ptr<indirect> indirect_;
	HWND window_;
	HDC hdc_;
	HGLRC context_;
	audio_engine *directsound_; // non-owning
	bool winquit_;
#endif
};

}

#endif
