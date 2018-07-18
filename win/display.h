#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

#include <functional>

namespace win
{

class display
{
	friend class system;

	typedef std::function<void(pkey, bool)> fn_event_keyboard_raw;
	typedef std::function<void(int, bool)> fn_event_keyboard_cooked;
	typedef std::function<void(int x, int y)> fn_event_mouse_move;
	typedef std::function<void(mouse, bool)> fn_event_mouse_click;

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
	void event_keyboard_raw(fn_event_keyboard_raw);
	void event_keyboard_cooked(fn_event_keyboard_cooked);
	void event_mouse_move(fn_event_mouse_move);
	void event_mouse_click(fn_event_mouse_click);

private:
	void move(display&);
	void finalize();

	struct
	{
		// keyboard
		fn_event_keyboard_raw keyboard_raw;
		fn_event_keyboard_cooked keyboard_cooked;

		// mouse
		fn_event_mouse_move mouse_move;
		fn_event_mouse_click mouse_click;
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
