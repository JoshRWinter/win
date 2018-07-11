#include <string.h>

#include "win.h"

win::display::display()
{
	memset(this, 0, sizeof(*this));
}

win::display::display(display &&rhs)
{
	move(rhs);
}

win::display &win::display::operator=(display &&rhs)
{
	finalize();
	move(rhs);
	return *this;
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

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

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

	int visual_attributes[] =
	{
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None
	};

	// get a frame buffer config
	int fb_count = 0;
	GLXFBConfig *fbconfig = glXChooseFBConfig(xdisplay, DefaultScreen(xdisplay), visual_attributes, &fb_count);
	if(fbconfig == NULL)
		throw exception("Could not find a suitable frame buffer configuration");

	// get a X visual config
	XVisualInfo *xvi = glXGetVisualFromFBConfig(xdisplay, fbconfig[0]);
	if(xvi == NULL)
		throw exception("Could not find a suitable X Visual configuration");

	// window settings
	XSetWindowAttributes xswa;
	xswa.colormap = XCreateColormap(xdisplay, RootWindow(xdisplay, xvi->screen), xvi->visual, AllocNone);
	xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	// create da window
	window_ = XCreateWindow(xdisplay, RootWindow(xdisplay, xvi->screen), 0, 0, width, height, 0, xvi->depth, InputOutput, xvi->visual, CWColormap | CWEventMask, &xswa);
	XMapWindow(xdisplay, window_);
	XStoreName(xdisplay, window_, caption);

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((unsigned char*)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB == NULL)
		throw exception("Could not find function glXCreateContextAttribsARB");

	const int context_attributes[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		None
	};

	// create opengl context
	context_ = glXCreateContextAttribsARB(xdisplay, fbconfig[0], NULL, true, context_attributes);
	if(context_ == None)
		throw exception("Could not create an OpenGL " + std::to_string(context_attributes[1]) + "." + std::to_string(context_attributes[3])  + " context");
	glXMakeCurrent(xdisplay, window_, context_);

	// set up delete window protocol
	wm_delete_window = XInternAtom(xdisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(xdisplay, window_, &wm_delete_window, 1);

	// vsync
	glXSwapIntervalEXT(xdisplay, window_, 1);

	XFree(xvi);
	XFree(fbconfig);
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

void win::display::move(display &rhs)
{
	window_ = rhs.window_;
	context_ = rhs.context_;

	rhs.window_ = None;
	rhs.context_ = NULL;
}

void win::display::finalize()
{
	if(window_ == 0)
		return;

	glXMakeCurrent(xdisplay, None, NULL);
	glXDestroyContext(xdisplay, context_);
	XDestroyWindow(xdisplay, window_);
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
