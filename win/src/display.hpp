#ifndef WIN_DISPLAY_HPP
#define WIN_DISPLAY_HPP

#include <functional>
#include <memory>

namespace win
{

class Display
{
	friend class SoundEngine;

	static void default_window_handler(WindowEvent) {}
	static void default_button_handler(Button, bool) {}
	static void default_character_handler(int) {}
	static void default_mouse_handler(int, int) {}

	typedef std::function<void(WindowEvent)> WindowHandler;
	typedef std::function<void(Button, bool)> ButtonHandler;
	typedef std::function<void(int)> CharacterHandler;
	typedef std::function<void(int x, int y)> MouseHandler;

public:
	Display(const char*, int, int, bool = false, window_handle = 0);
	Display(const Display&) = delete;
	Display(Display&&) = delete;
	~Display();

	Display &operator=(Display&&) = delete;
	Display &operator=(Display&) = delete;

	void process();
	void swap();
	int width();
	int height();
	void cursor(bool);
	void vsync(bool);

	void register_window_handler(WindowHandler);
	void register_button_handler(ButtonHandler);
	void register_character_handler(CharacterHandler);
	void register_mouse_handler(MouseHandler);

	static int screen_width();
	static int screen_height();

private:
	WindowHandler window_handler;
	ButtonHandler button_handler;
	CharacterHandler character_handler;
	MouseHandler mouse_handler;

#if defined WINPLAT_LINUX
	Window window;
	GLXContext context;
#elif defined WINPLAT_WINDOWS
	HWND window;
	HDC hdc;
	HGLRC context;

	static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	static void win_init_gl(Display&, HWND);
	void win_term_gl();
#endif
};

}

#endif
