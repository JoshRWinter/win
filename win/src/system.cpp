#include <win.h>

static win::system *singleton = NULL;

win::system::system()
{
	if(singleton != NULL)
		throw exception("Existing system instance");

	singleton = this;
}

win::system::system(system&&)
{
	singleton = this;
}

win::system::~system()
{
	if(singleton == this)
		singleton = NULL;
}

win::system &win::system::operator=(system&&)
{
	singleton = this;

	return *this;
}

win::display win::system::make_display(const char *caption, int width, int height, int flags, window_handle parent)
{
	return display(caption, width, height, flags, parent);
}

win::system &win::system::get()
{
	return *singleton;
}
