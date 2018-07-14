#include <string.h>

#include "win.h"

// default event handlers
static void handler_keyboard(int, int, bool) {}
static void handler_mouse_move(int, int) {}
static void handler_mouse_click(win::mouse, bool) {}

win::display::display()
{
#if defined WINPLAT_LINUX
	window_ = None;
	context_ = NULL;
#endif
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

static int x_get_key(int keycode)
{
	int count = 0;
	KeySym *sym = XGetKeyboardMapping(xdisplay, keycode, 1, &count);
	const int result = sym[0];
	XFree(sym);

	return result > 127 ? 0 : result;
}

win::display::display(const char *caption, int width, int height, int flags, window_handle parent)
{
	handler.keyboard = handler_keyboard;
	handler.mouse_move = handler_mouse_move;
	handler.mouse_click = handler_mouse_click;

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

// return false if application is to exit
bool win::display::process()
{
	while(XPending(xdisplay))
	{
		XEvent xevent;
		XPeekEvent(xdisplay, &xevent);
		if(xevent.xany.window != window_)
			return true;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
			case ClientMessage: return false;

			case KeyPress:
				handler.keyboard(xevent.xkey.keycode, x_get_key(xevent.xkey.keycode), true);
				break;
			case KeyRelease:
			{
				XEvent e;
				if(XPending(xdisplay))
				{
					XPeekEvent(xdisplay, &e);
					if(xevent.xkey.keycode == e.xkey.keycode && e.type == KeyPress)
						break;
				}
				handler.keyboard(xevent.xkey.keycode, x_get_key(xevent.xkey.keycode), false);
				break;
			}
		}
	}

	return true;
}

void win::display::swap() const
{
	glXSwapBuffers(xdisplay, window_);
}

void win::display::event_keyboard(fn_event_keyboard f)
{
	handler.keyboard = std::move(f);
}

void win::display::event_mouse_move(fn_event_mouse_move f)
{
	handler.mouse_move = std::move(f);
}

void win::display::event_mouse_click(fn_event_mouse_click f)
{
	handler.mouse_click = std::move(f);
}

void win::display::move(display &rhs)
{
	handler.keyboard = std::move(rhs.handler.keyboard);
	handler.mouse_move = std::move(rhs.handler.mouse_move);
	handler.mouse_click = std::move(rhs.handler.mouse_click);

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
