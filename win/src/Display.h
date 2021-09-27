#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

#include <functional>
#include <memory>

namespace win
{

class Display
{
	friend class SoundEngine;

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

	bool process();
	void swap();
	int width();
	int height();
	void cursor(bool);
	void vsync(bool);

	void register_button_handler(ButtonHandler);
	void register_character_handler(CharacterHandler);
	void register_mouse_handler(MouseHandler);

	static int screen_width();
	static int screen_height();

private:
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
	bool winquit;

	static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	static void win_init_gl(Display&, HWND);
	void win_term_gl();
#endif
};

}

#endif
