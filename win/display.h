#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

namespace win
{

class display
{
	friend class system;

public:
	display();
	display(const char*, int, int, int, window_handle);
	display(const display&) = delete;
	display(display&&);
	~display();

	display &operator=(display&&);

	event poll();

private:
	void finalize();

#if defined WINPLAT_LINUX
	Window window_;
	GLXContext context_;
	XVisualInfo *xvi_;
#elif defined WINPLAT_WINDOWS
#else
#error "unsupported platform"
#endif
};

}

#endif
