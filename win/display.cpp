#include "win.h"

win::display::display(const char *caption, int width, int height, int flags, window_handle parent)
{
}

win::display::display(display &&rhs)
{
	window = rhs.window;
	rhs.window = NULL;
}

win::display &win::display::operator=(display &&rhs)
{
	window = rhs.window;
	rhs.window = NULL;
	return *this;
}
