#include <string.h>

#include "win.h"

win::display::display()
{
	memset(this, 0, sizeof(*this));
}

win::display::display(display &&rhs)
{
#if defined WINPLAT_LINUX
	window_ = rhs.window_;
	context_ = rhs.context_;
	xvi_ = rhs.xvi_;

	rhs.window_ = 0;
	rhs.context_ = NULL;
	rhs.xvi_ = NULL;
#endif
}

win::display &win::display::operator=(display &&rhs)
{
	finalize();

#if defined WINPLAT_LINUX
	window_ = rhs.window_;
	context_ = rhs.context_;
	xvi_ = rhs.xvi_;

	rhs.window_ = 0;
	rhs.context_ = NULL;
	rhs.xvi_ = NULL;

	return *this;
#endif
}

win::display::~display()
{
	finalize();
}

/* ------------------------------------*/
/////////////////////////////////////////
///// LINUX /////////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

#if defined WINPLAT_LINUX

static Display *xdisplay;
static Atom wm_delete_window;
struct x_display_helper
{
	x_display_helper()
	{
		xdisplay = XOpenDisplay(NULL);
	}

	~x_display_helper()
	{
		XCloseDisplay(xdisplay);
	}
} xdisp;

win::display::display(const char *caption, int width, int height, int flags, window_handle parent)
{
	if(xdisplay == NULL)
		throw exception("Could not connect to the X server");

	load_extensions();

	int attributes[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
	None };

	xvi_ = glXChooseVisual(xdisplay, 0, attributes);
	if(xvi_ == NULL)
		throw exception("no appropriate X visuals could be found");

	Window root = DefaultRootWindow(xdisplay);

	Colormap cmap = XCreateColormap(xdisplay, root, xvi_->visual, AllocNone);

	XSetWindowAttributes xswa;
	xswa.colormap = cmap;
	xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	window_ = XCreateWindow(xdisplay, root, 0, 0, width, height, 0, xvi_->depth, InputOutput, xvi_->visual, CWColormap | CWEventMask, &xswa);

	XMapWindow(xdisplay, window_);
	XStoreName(xdisplay, window_, caption);

	context_ = glXCreateContext(xdisplay, xvi_, NULL, GL_TRUE);
	glXMakeCurrent(xdisplay, window_, context_);

	// set up delete window protocol
	wm_delete_window = XInternAtom(xdisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(xdisplay, window_, &wm_delete_window, 1);

	// vsync
	glXSwapIntervalEXT(xdisplay, window_, 1);
}

win::event win::display::poll()
{
	if(!XPending(xdisplay))
		return event::NONE;

	XEvent xevent;
	XNextEvent(xdisplay, &xevent);

	switch(xevent.type)
	{
		case ClientMessage: return event::CLOSE;
		default: return event::NONE;
	}

	return event::NONE;
}

void win::display::swap() const
{
	glXSwapBuffers(xdisplay, window_);
}

void win::display::finalize()
{
	if(window_ == 0)
		return;

	glXMakeCurrent(xdisplay, None, NULL);
	glXDestroyContext(xdisplay, context_);
	XDestroyWindow(xdisplay, window_);
	XFree(xvi_);
}

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/
#elif defined WINPLAT_WINDOWS

#else
#error "unsupported platform"
#endif
