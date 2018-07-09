#ifndef WIN_DISPLAY_H
#define WIN_DISPLAY_H

namespace win
{

class display
{
	friend class system;

public:
	display() : window(NULL) {}
	display(const char*, int, int, int, window_handle);
	display(const display&) = delete;
	display(display&&);

	display &operator=(display&&);

private:
	window_handle window;
};

}

#endif
