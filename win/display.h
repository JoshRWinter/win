#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

#include <functional>

namespace win
{

class display
{
	friend class system;

	typedef std::function<void(button, bool)> fn_event_button;
	typedef std::function<void(int, bool)> fn_event_character;
	typedef std::function<void(int x, int y)> fn_event_mouse;

public:
	static constexpr int FULLSCREEN = 1;

	display();
	display(const char*, int, int, int, window_handle);
	display(const display&) = delete;
	display(display&&);
	~display();

	display &operator=(display&&);

	bool process();
	void swap() const;
	int width() const;
	int height() const;

	void event_button(fn_event_button);
	void event_character(fn_event_character);
	void event_mouse(fn_event_mouse);

	static int screen_width();
	static int screen_height();

private:
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
#elif defined WINPLAT_WINDOWS
#else
#error "unsupported platform"
#endif
};

}

#endif
