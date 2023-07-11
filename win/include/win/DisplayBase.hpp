#pragma once

#include <functional>

#include <win/Win.hpp>
#include <win/Event.hpp>

#ifdef WINPLAT_LINUX
#include <X11/Xlib.h>
#endif

namespace win
{

#if defined WINPLAT_WINDOWS
typedef HWND NativeWindowHandle;
#elif defined WINPLAT_LINUX
typedef Window* NativeWindowHandle;
#endif

struct DisplayOptions
{
	DisplayOptions()
		: fullscreen(false)
		  , width(800)
		  , height(600)
		  , gl_major(0)
		  , gl_minor(0)
		  , debug(false)
		  , parent(NULL)
	{}

	std::string caption;
	bool fullscreen;
	int width;
	int height;
	int gl_major;
	int gl_minor;
	bool debug;
	NativeWindowHandle parent;
};

class DisplayBase
{
	WIN_NO_COPY_MOVE(DisplayBase);

	static void default_window_handler(WindowEvent event) {}
	static void default_button_handler(Button button, bool press) {}
	static void default_character_handler(int c) {}
	static void default_mouse_handler(int x, int y) {}

public:
	typedef std::function<void(WindowEvent event)> WindowHandler;
	typedef std::function<void(Button button, bool press)> ButtonHandler;
	typedef std::function<void(int c)> CharacterHandler;
	typedef std::function<void(int x, int y)> MouseHandler;

	DisplayBase();
	virtual ~DisplayBase() = default;

	virtual void process() = 0;
	virtual void swap() = 0;
	virtual int width() = 0;
	virtual int height() = 0;
	virtual int screen_width() = 0;
	virtual int screen_height() = 0;
	virtual void cursor(bool) = 0;
	virtual void vsync(bool) = 0;
	virtual NativeWindowHandle native_handle() = 0;

	void register_window_handler(WindowHandler);
	void register_button_handler(ButtonHandler);
	void register_character_handler(CharacterHandler);
	void register_mouse_handler(MouseHandler);

protected:
	WindowHandler window_handler;
	ButtonHandler button_handler;
	CharacterHandler character_handler;
	MouseHandler mouse_handler;
};

}
