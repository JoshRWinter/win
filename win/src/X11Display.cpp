#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>

#include <win/X11Display.hpp>
#include <win/X11MonitorEnumerator.hpp>

typedef GLXContext (*glXCreateContextAttribsARBProc)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);

static Atom atom_delete_window;
static Atom atom_fullscreen;
static Atom atom_wm_state;
static ::Display *xdisplay;
static XkbDescPtr xkb_desc;

static struct x_init_helper
{
	x_init_helper()
	{
		int event_return = 0;
		int error_return = 0;
		int major_in_out = XkbMajorVersion;
		int minor_in_out = XkbMinorVersion;
		int reason_return = 0;
		xdisplay = XkbOpenDisplay(NULL, &event_return, &error_return, &major_in_out, &minor_in_out, &reason_return);

		// x keyboard extension schtuff
		xkb_desc = XkbGetMap(xdisplay, 0, XkbUseCoreKbd);
		if(xkb_desc == NULL)
			win::bug("Could not get the X keyboard map");
		XkbGetNames(xdisplay, XkbKeyNamesMask, xkb_desc);

		// window close message
		atom_delete_window = XInternAtom(xdisplay, "WM_DELETE_WINDOW", False);

		// fullscreen atom
		atom_fullscreen = XInternAtom(xdisplay, "_NET_WM_STATE_FULLSCREEN", False);

		// wm state atom
		atom_wm_state = XInternAtom(xdisplay, "_NET_WM_STATE", False);
	}

	~x_init_helper()
	{
		XkbFreeKeyboard(xkb_desc, 0, True);
		XCloseDisplay(xdisplay);
	}
} xinit_helper;

static int x_get_keysym(XKeyEvent *event)
{
	char buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	KeySym sym = 0;
	if(!XLookupString(event, buffer, sizeof(buffer), &sym, NULL))
		return 0;

	return sym < ' ' || sym > '~' ? 0 : sym;
}

static std::string name_to_string(const char* name)
{
	char str[5] = {name[0], name[1], name[2], name[3], 0};
	return str;
}

constexpr int ascii_append(const char *const source, const int index = 0, int number = 0)
{
	if(index > 3)
		return number;

	if(source[index] == '\0')
		return number;

	const char c = source[index] - '0';

	const int first = c / 10;
	const int second = c % 10;

	number *= 10;
	number += first;
	number *= 10;
	number += second;

	return ascii_append(source, index + 1, number);
}

constexpr int keystring_hash(const char *const keycode_hash)
{
	return ascii_append(keycode_hash);
}

static win::Button keystring_to_button(const char *const keystring)
{
	switch(keystring_hash(keystring))
	{
		case keystring_hash("MS1\0"): return win::Button::mouse_left;
		case keystring_hash("MS2\0"): return win::Button::mouse_right;
		case keystring_hash("MS3\0"): return win::Button::mouse_middle;
		case keystring_hash("MS4\0"): return win::Button::mouse4;
		case keystring_hash("MS5\0"): return win::Button::mouse5;
		case keystring_hash("MS6\0"): return win::Button::mouse6;
		case keystring_hash("MS7\0"): return win::Button::mouse7;

		case keystring_hash("AC01"): return win::Button::a;
		case keystring_hash("AB05"): return win::Button::b;
		case keystring_hash("AB03"): return win::Button::c;
		case keystring_hash("AC03"): return win::Button::d;
		case keystring_hash("AD03"): return win::Button::e;
		case keystring_hash("AC04"): return win::Button::f;
		case keystring_hash("AC05"): return win::Button::g;
		case keystring_hash("AC06"): return win::Button::h;
		case keystring_hash("AD08"): return win::Button::i;
		case keystring_hash("AC07"): return win::Button::j;
		case keystring_hash("AC08"): return win::Button::k;
		case keystring_hash("AC09"): return win::Button::l;
		case keystring_hash("AB07"): return win::Button::m;
		case keystring_hash("AB06"): return win::Button::n;
		case keystring_hash("AD09"): return win::Button::o;
		case keystring_hash("AD10"): return win::Button::p;
		case keystring_hash("AD01"): return win::Button::q;
		case keystring_hash("AD04"): return win::Button::r;
		case keystring_hash("AC02"): return win::Button::s;
		case keystring_hash("AD05"): return win::Button::t;
		case keystring_hash("AD07"): return win::Button::u;
		case keystring_hash("AB04"): return win::Button::v;
		case keystring_hash("AD02"): return win::Button::w;
		case keystring_hash("AB02"): return win::Button::x;
		case keystring_hash("AD06"): return win::Button::y;
		case keystring_hash("AB01"): return win::Button::z;

		case keystring_hash("AE10"): return win::Button::d0;
		case keystring_hash("AE01"): return win::Button::d1;
		case keystring_hash("AE02"): return win::Button::d2;
		case keystring_hash("AE03"): return win::Button::d3;
		case keystring_hash("AE04"): return win::Button::d4;
		case keystring_hash("AE05"): return win::Button::d5;
		case keystring_hash("AE06"): return win::Button::d6;
		case keystring_hash("AE07"): return win::Button::d7;
		case keystring_hash("AE08"): return win::Button::d8;
		case keystring_hash("AE09"): return win::Button::d9;

		case keystring_hash("TLDE"): return win::Button::backtick;
		case keystring_hash("AE11"): return win::Button::dash;
		case keystring_hash("AE12"): return win::Button::equals;
		case keystring_hash("AD11"): return win::Button::lbracket;
		case keystring_hash("AD12"): return win::Button::rbracket;
		case keystring_hash("AC10"): return win::Button::semicolon;
		case keystring_hash("AC11"): return win::Button::apostrophe;
		case keystring_hash("AB08"): return win::Button::comma;
		case keystring_hash("AB09"): return win::Button::period;
		case keystring_hash("AB10"): return win::Button::slash;
		case keystring_hash("BKSL"): return win::Button::backslash;

		case keystring_hash("FK01"): return win::Button::f1;
		case keystring_hash("FK02"): return win::Button::f2;
		case keystring_hash("FK03"): return win::Button::f3;
		case keystring_hash("FK04"): return win::Button::f4;
		case keystring_hash("FK05"): return win::Button::f5;
		case keystring_hash("FK06"): return win::Button::f6;
		case keystring_hash("FK07"): return win::Button::f7;
		case keystring_hash("FK08"): return win::Button::f8;
		case keystring_hash("FK09"): return win::Button::f9;
		case keystring_hash("FK10"): return win::Button::f10;
		case keystring_hash("FK11"): return win::Button::f11;
		case keystring_hash("FK12"): return win::Button::f12;

		case keystring_hash("ESC\0"): return win::Button::esc;
		case keystring_hash("PRSC"): return win::Button::print_scr;
		case keystring_hash("PAUS"): return win::Button::pause_break;
		case keystring_hash("INS\0"): return win::Button::insert;
		case keystring_hash("DELE"): return win::Button::del;
		case keystring_hash("HOME"): return win::Button::home;
		case keystring_hash("PGUP"): return win::Button::page_up;
		case keystring_hash("PGDN"): return win::Button::page_down;
		case keystring_hash("END\0"): return win::Button::end;
		case keystring_hash("BKSP"): return win::Button::backspace;
		case keystring_hash("RTRN"): return win::Button::ret;
		case keystring_hash("KPEN"): return win::Button::enter;
		case keystring_hash("LFSH"): return win::Button::lshift;
		case keystring_hash("RTSH"): return win::Button::rshift;
		case keystring_hash("LCTL"): return win::Button::lctrl;
		case keystring_hash("RCTL"): return win::Button::rctrl;
		case keystring_hash("LALT"): return win::Button::lalt;
		case keystring_hash("RALT"): return win::Button::ralt;
		case keystring_hash("SPCE"): return win::Button::space;
		case keystring_hash("COMP"): return win::Button::menu;
		case keystring_hash("LWIN"): return win::Button::lmeta;
		case keystring_hash("RWIN"): return win::Button::rmeta;

		case keystring_hash("UP\0\0"): return win::Button::up;
		case keystring_hash("LEFT"): return win::Button::left;
		case keystring_hash("RGHT"): return win::Button::right;
		case keystring_hash("DOWN"): return win::Button::down;

		case keystring_hash("CAPS"): return win::Button::capslock;
		case keystring_hash("TAB\0"): return win::Button::tab;
		case keystring_hash("NMLK"): return win::Button::num_lock;
		case keystring_hash("KPDV"): return win::Button::num_slash;
		case keystring_hash("KPMU"): return win::Button::num_multiply;
		case keystring_hash("KPSU"): return win::Button::num_minus;
		case keystring_hash("KPAD"): return win::Button::num_plus;
		case keystring_hash("KPDL"): return win::Button::num_del;

		case keystring_hash("KP0\0"): return win::Button::num0;
		case keystring_hash("KP1\0"): return win::Button::num1;
		case keystring_hash("KP2\0"): return win::Button::num2;
		case keystring_hash("KP3\0"): return win::Button::num3;
		case keystring_hash("KP4\0"): return win::Button::num4;
		case keystring_hash("KP5\0"): return win::Button::num5;
		case keystring_hash("KP6\0"): return win::Button::num6;
		case keystring_hash("KP7\0"): return win::Button::num7;
		case keystring_hash("KP8\0"): return win::Button::num8;
		case keystring_hash("KP9\0"): return win::Button::num9;

		default: return win::Button::undefined;
	}
}

namespace win
{

X11Display::X11Display(const DisplayOptions &options)
	: options(options)
{
	if (options.width < 1 || options.height < 1)
		win::bug("Invalid window dimensions");
	if (options.gl_major == 0)
		win::bug("Unsupported GL version");

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
		win::bug("Could not find a suitable frame buffer configuration");

	// get a X visual config
	XVisualInfo *xvi = glXGetVisualFromFBConfig(xdisplay, fbconfig[0]);
	if(xvi == NULL)
		win::bug("Could not find a suitable X Visual configuration");

	// window settings
	XSetWindowAttributes xswa;
	xswa.colormap = XCreateColormap(xdisplay, RootWindow(xdisplay, xvi->screen), xvi->visual, AllocNone);
	xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

	X11MonitorEnumerator monitors;
	const auto &monitor = monitors[0];

	// create da window
	window = XCreateWindow(
		xdisplay,
		RootWindow(xdisplay, xvi->screen),
		options.fullscreen ? 0 : (monitor.x + (monitor.width / 2)) - (options.width / 2),
		options.fullscreen ? 0 : (monitor.y + (monitor.height / 2)) - (options.height / 2),
		options.fullscreen ? monitor.width : options.width,
		options.fullscreen ? monitor.height : options.height,
		0,
		xvi->depth,
		InputOutput,
		xvi->visual,
		CWColormap | CWEventMask,
		&xswa
	);

	XMapWindow(xdisplay, window);
	XStoreName(xdisplay, window, options.caption.c_str());

	// fullscreen
	if(options.fullscreen)
		XChangeProperty(xdisplay, window, atom_wm_state, XA_ATOM, 32, PropModeReplace, (const unsigned char*)&atom_fullscreen, 1);

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((unsigned char*)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB == NULL)
		win::bug("Could not find function glXCreateContextAttribsARB");

	const int context_attributes[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, options.gl_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, options.gl_minor,
		options.debug ? GLX_CONTEXT_FLAGS_ARB : GL_NONE, options.debug ? GLX_CONTEXT_DEBUG_BIT_ARB : GL_NONE,
		None
	};

	// create opengl context
	context = glXCreateContextAttribsARB(xdisplay, fbconfig[0], NULL, true, context_attributes);
	if(context == None)
		win::bug("Could not create an OpenGL " + std::to_string(context_attributes[1]) + "." + std::to_string(context_attributes[3]) + " context");
	glXMakeCurrent(xdisplay, window, context);

	// set up delete window protocol
	XSetWMProtocols(xdisplay, window, &atom_delete_window, 1);

	// vsync
	glXSwapIntervalEXT = (decltype(glXSwapIntervalEXT))glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
	glXSwapIntervalEXT(xdisplay, window, 1);

	XFree(xvi);
	XFree(fbconfig);

	update_refresh_rate();

	window_prop_cache.w = options.fullscreen ? monitor.width : options.width;
	window_prop_cache.h = options.fullscreen ? monitor.height : options.height;
}

X11Display::~X11Display()
{
	glXMakeCurrent(xdisplay, None, NULL);
	glXDestroyContext(xdisplay, context);
	XDestroyWindow(xdisplay, window);
}

void X11Display::process()
{
	if (resize_state.resized && std::chrono::duration<float>(std::chrono::steady_clock::now() - resize_state.time).count() > 0.25f)
	{
		resize_state.resized = false;
		resize_handler(window_prop_cache.w, window_prop_cache.h);
	}

	while(XPending(xdisplay))
	{
		XEvent xevent;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
			case ClientMessage:
				window_handler(win::WindowEvent::close);
				break;
			case ConfigureNotify:
			{
				Window root;
				int x, y;
				unsigned w, h, b, d;
				XGetGeometry(xdisplay, window, &root, &x, &y, &w, &h, &b, &d);

				const bool resized = w != window_prop_cache.w || h != window_prop_cache.h;
				const bool moved = x != window_prop_cache.x || y != window_prop_cache.y;

				if (moved || resized)
					update_refresh_rate();

				if (resized)
				{
					resize_state.resized = true;
					resize_state.time = std::chrono::steady_clock::now();
				}

				window_prop_cache.x = x;
				window_prop_cache.y = y;
				window_prop_cache.w = w;
				window_prop_cache.h = h;
			}
			break;
			case KeyPress:
			{
				button_handler(keystring_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), true);
				const KeySym sym = x_get_keysym(&xevent.xkey);
				if(sym)
					character_handler(sym);
				break;
			}
			case KeyRelease:
			{
				XEvent e;
				if(XPending(xdisplay))
				{
					XPeekEvent(xdisplay, &e);
					if(e.xany.window == xevent.xany.window && xevent.xkey.keycode == e.xkey.keycode && e.type == KeyPress)
						break;
				}

				button_handler(keystring_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), false);
				break;
			}
			case MotionNotify:
				mouse_handler(xevent.xmotion.x, xevent.xmotion.y);
				break;
			case ButtonPress:
			case ButtonRelease:
				switch(xevent.xbutton.button)
				{
					case 1:
						button_handler(Button::mouse_left, xevent.type == ButtonPress);
						break;
					case 2:
						button_handler(Button::mouse_middle, xevent.type == ButtonPress);
						break;
					case 3:
						button_handler(Button::mouse_right, xevent.type == ButtonPress);
						break;
				}
				break;
		}
	}
}

void X11Display::swap()
{
	glXSwapBuffers(xdisplay, window);
}

int X11Display::width()
{
	Window root;
	int xpos;
	int ypos;
	unsigned width = 0;
	unsigned height;
	unsigned border;
	unsigned depth;

	XGetGeometry(xdisplay, window, &root, &xpos, &ypos, &width, &height, &border, &depth);

	return width;
}

int X11Display::height()
{
	Window root;
	int xpos;
	int ypos;
	unsigned width;
	unsigned height = 0;
	unsigned border;
	unsigned depth;

	XGetGeometry(xdisplay, window, &root, &xpos, &ypos, &width, &height, &border, &depth);

	return height;
}

void X11Display::resize(int w, int h)
{
	XResizeWindow(xdisplay, window, w, h);
}

float X11Display::refresh_rate()
{
	return rrate;
}

void X11Display::cursor(bool show)
{
	if(show)
	{
		XUndefineCursor(xdisplay, window);
	}
	else
	{
		Pixmap pm;
		XColor dummy;
		char data[2] = {0, 0};
		Cursor cursor;

		pm = XCreateBitmapFromData(xdisplay, window, data, 1, 1);
		cursor = XCreatePixmapCursor(xdisplay, pm, pm, &dummy, &dummy, 0, 0);
		XFreePixmap(xdisplay, pm);

		XDefineCursor(xdisplay, window, cursor);
	}
}

void X11Display::vsync(bool on)
{
	glXSwapIntervalEXT(xdisplay, window, on);
}

void X11Display::set_fullscreen(bool fullscreen)
{
	int monx, mony, monw, monh;
	float monrate;
	get_current_monitor_props(monx, mony, monw, monh, monrate);

	XMoveResizeWindow(
		xdisplay,
		window,
		fullscreen ? monx : ((monx + (monw / 2)) - (options.width / 2)),
		fullscreen ? mony : ((mony + (monh / 2)) - (options.height / 2)),
		fullscreen ? monw : options.width,
		fullscreen ? monh : options.height);

	XChangeProperty(xdisplay, window, atom_wm_state, XA_ATOM, 32, PropModeReplace, (const unsigned char *)&atom_fullscreen, fullscreen);
}

NativeWindowHandle X11Display::native_handle()
{
	return &window;
}

void X11Display::update_refresh_rate()
{
	int x, y, w, h;
	float rr;

	get_current_monitor_props(x, y, w, h, rr);

	rrate = rr;
}

void X11Display::get_current_monitor_props(int &x, int &y, int &w, int &h, float &rr)
{
	x = 0;
	y = 0;
	w = 0;
	h = 0;
	rr = 0;

	Window root;

	int windowx, windowy;
	unsigned windoww, windowh, b, d;

	XGetGeometry(xdisplay, window, &root, &windowx, &windowy, &windoww, &windowh, &b, &d);

	X11MonitorEnumerator monitors;
	for (const auto &monitor : monitors)
	{
		if (contains_point(monitor.x, monitor.y, monitor.width, monitor.height, windowx + (windoww / 2), windowy + (windowh / 2)))
		{
			mon_props_cache.x = monitor.x;
			mon_props_cache.y = monitor.y;
			mon_props_cache.w = monitor.width;
			mon_props_cache.h = monitor.height;
			mon_props_cache.rate = monitor.rate;

			break;
		}
	}

	x = mon_props_cache.x;
	y = mon_props_cache.y;
	w = mon_props_cache.w;
	h = mon_props_cache.h;
	rr = mon_props_cache.rate;
}

bool X11Display::contains_point(int monitorx, int monitory, int monitorw, int monitorh, int x, int y)
{
	return x >= monitorx && x < monitorx + monitorw && y >= monitory && y < monitory + monitorh;
}

}

#endif
