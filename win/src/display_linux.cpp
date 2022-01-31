#include <win.h>

#ifdef WINPLAT_LINUX

#include <X11/Xatom.h>

typedef GLXContext (*glXCreateContextAttribsARBProc)(::Display*, GLXFBConfig, GLXContext, Bool, const int*);

static Atom atom_delete_window;
static Atom atom_fullscreen;
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
	case keystring_hash("MS1\0"): return win::Button::MOUSE_LEFT;
	case keystring_hash("MS2\0"): return win::Button::MOUSE_RIGHT;
	case keystring_hash("MS3\0"): return win::Button::MOUSE_MIDDLE;
	case keystring_hash("MS4\0"): return win::Button::MOUSE4;
	case keystring_hash("MS5\0"): return win::Button::MOUSE5;
	case keystring_hash("MS6\0"): return win::Button::MOUSE6;
	case keystring_hash("MS7\0"): return win::Button::MOUSE7;

	case keystring_hash("AC01"): return win::Button::A;
	case keystring_hash("AB05"): return win::Button::B;
	case keystring_hash("AB03"): return win::Button::C;
	case keystring_hash("AC03"): return win::Button::D;
	case keystring_hash("AD03"): return win::Button::E;
	case keystring_hash("AC04"): return win::Button::F;
	case keystring_hash("AC05"): return win::Button::G;
	case keystring_hash("AC06"): return win::Button::H;
	case keystring_hash("AD08"): return win::Button::I;
	case keystring_hash("AC07"): return win::Button::J;
	case keystring_hash("AC08"): return win::Button::K;
	case keystring_hash("AC09"): return win::Button::L;
	case keystring_hash("AB07"): return win::Button::M;
	case keystring_hash("AB06"): return win::Button::N;
	case keystring_hash("AD09"): return win::Button::O;
	case keystring_hash("AD10"): return win::Button::P;
	case keystring_hash("AD01"): return win::Button::Q;
	case keystring_hash("AD04"): return win::Button::R;
	case keystring_hash("AC02"): return win::Button::S;
	case keystring_hash("AD05"): return win::Button::T;
	case keystring_hash("AD07"): return win::Button::U;
	case keystring_hash("AB04"): return win::Button::V;
	case keystring_hash("AD02"): return win::Button::W;
	case keystring_hash("AB02"): return win::Button::X;
	case keystring_hash("AD06"): return win::Button::Y;
	case keystring_hash("AB01"): return win::Button::Z;

	case keystring_hash("AE10"): return win::Button::D0;
	case keystring_hash("AE01"): return win::Button::D1;
	case keystring_hash("AE02"): return win::Button::D2;
	case keystring_hash("AE03"): return win::Button::D3;
	case keystring_hash("AE04"): return win::Button::D4;
	case keystring_hash("AE05"): return win::Button::D5;
	case keystring_hash("AE06"): return win::Button::D6;
	case keystring_hash("AE07"): return win::Button::D7;
	case keystring_hash("AE08"): return win::Button::D8;
	case keystring_hash("AE09"): return win::Button::D9;

	case keystring_hash("TLDE"): return win::Button::BACKTICK;
	case keystring_hash("AE11"): return win::Button::DASH;
	case keystring_hash("AE12"): return win::Button::EQUALS;
	case keystring_hash("AD11"): return win::Button::LBRACKET;
	case keystring_hash("AD12"): return win::Button::RBRACKET;
	case keystring_hash("AC10"): return win::Button::SEMICOLON;
	case keystring_hash("AC11"): return win::Button::APOSTROPHE;
	case keystring_hash("AB08"): return win::Button::COMMA;
	case keystring_hash("AB09"): return win::Button::PERIOD;
	case keystring_hash("AB10"): return win::Button::SLASH;
	case keystring_hash("BKSL"): return win::Button::BACKSLASH;

	case keystring_hash("FK01"): return win::Button::F1;
	case keystring_hash("FK02"): return win::Button::F2;
	case keystring_hash("FK03"): return win::Button::F3;
	case keystring_hash("FK04"): return win::Button::F4;
	case keystring_hash("FK05"): return win::Button::F5;
	case keystring_hash("FK06"): return win::Button::F6;
	case keystring_hash("FK07"): return win::Button::F7;
	case keystring_hash("FK08"): return win::Button::F8;
	case keystring_hash("FK09"): return win::Button::F9;
	case keystring_hash("FK10"): return win::Button::F10;
	case keystring_hash("FK11"): return win::Button::F11;
	case keystring_hash("FK12"): return win::Button::F12;

	case keystring_hash("ESC\0"): return win::Button::ESC;
	case keystring_hash("PRSC"): return win::Button::PRINT_SCR;
	case keystring_hash("PAUS"): return win::Button::PAUSE_BREAK;
	case keystring_hash("INS\0"): return win::Button::INSERT;
	case keystring_hash("DELE"): return win::Button::DELETE;
	case keystring_hash("HOME"): return win::Button::HOME;
	case keystring_hash("PGUP"): return win::Button::PAGE_UP;
	case keystring_hash("PGDN"): return win::Button::PAGE_DOWN;
	case keystring_hash("END\0"): return win::Button::END;
	case keystring_hash("BKSP"): return win::Button::BACKSPACE;
	case keystring_hash("RTRN"): return win::Button::RETURN;
	case keystring_hash("KPEN"): return win::Button::ENTER;
	case keystring_hash("LFSH"): return win::Button::LSHIFT;
	case keystring_hash("RTSH"): return win::Button::RSHIFT;
	case keystring_hash("LCTL"): return win::Button::LCTRL;
	case keystring_hash("RCTL"): return win::Button::RCTRL;
	case keystring_hash("LALT"): return win::Button::LALT;
	case keystring_hash("RALT"): return win::Button::RALT;
	case keystring_hash("SPCE"): return win::Button::SPACE;
	case keystring_hash("COMP"): return win::Button::MENU;
	case keystring_hash("LWIN"): return win::Button::LMETA;
	case keystring_hash("RWIN"): return win::Button::RMETA;

	case keystring_hash("UP\0\0"): return win::Button::UP;
	case keystring_hash("LEFT"): return win::Button::LEFT;
	case keystring_hash("RGHT"): return win::Button::RIGHT;
	case keystring_hash("DOWN"): return win::Button::DOWN;

	case keystring_hash("CAPS"): return win::Button::CAPSLOCK;
	case keystring_hash("TAB\0"): return win::Button::TAB;
	case keystring_hash("NMLK"): return win::Button::NUM_LOCK;
	case keystring_hash("KPDV"): return win::Button::NUM_SLASH;
	case keystring_hash("KPMU"): return win::Button::NUM_MULTIPLY;
	case keystring_hash("KPSU"): return win::Button::NUM_MINUS;
	case keystring_hash("KPAD"): return win::Button::NUM_PLUS;
	case keystring_hash("KPDL"): return win::Button::NUM_DEL;

	case keystring_hash("KP0\0"): return win::Button::NUM0;
	case keystring_hash("KP1\0"): return win::Button::NUM1;
	case keystring_hash("KP2\0"): return win::Button::NUM2;
	case keystring_hash("KP3\0"): return win::Button::NUM3;
	case keystring_hash("KP4\0"): return win::Button::NUM4;
	case keystring_hash("KP5\0"): return win::Button::NUM5;
	case keystring_hash("KP6\0"): return win::Button::NUM6;
	case keystring_hash("KP7\0"): return win::Button::NUM7;
	case keystring_hash("KP8\0"): return win::Button::NUM8;
	case keystring_hash("KP9\0"): return win::Button::NUM9;

	default: return win::Button::UNDEFINED;
	}
}

namespace win
{

Display::Display(const DisplayOptions &options)
{
	if (options.width < 1 || options.height < 1)
		win::bug("Invalid window dimensions");
	if (options.gl_major == 0)
		win::bug("Unsupported GL version");

	window_handler = default_window_handler;
	button_handler = default_button_handler;
	character_handler = default_character_handler;
	mouse_handler = default_mouse_handler;
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
		win::bug("Could not find a suitable frame buffer configuration");

	// get a X visual config
	XVisualInfo *xvi = glXGetVisualFromFBConfig(xdisplay, fbconfig[0]);
	if(xvi == NULL)
		win::bug("Could not find a suitable X Visual configuration");

	// window settings
	XSetWindowAttributes xswa;
	xswa.colormap = XCreateColormap(xdisplay, RootWindow(xdisplay, xvi->screen), xvi->visual, AllocNone);
	xswa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	// create da window
	window = XCreateWindow(xdisplay, RootWindow(xdisplay, xvi->screen), 0, 0, options.width, options.height, 0, xvi->depth, InputOutput, xvi->visual, CWColormap | CWEventMask, &xswa);
	XMapWindow(xdisplay, window);
	XStoreName(xdisplay, window, options.caption.c_str());

	// fullscreen
	if(options.fullscreen)
		XChangeProperty(xdisplay, window, XInternAtom(xdisplay, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (const unsigned char*)&atom_fullscreen, 1);

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((unsigned char*)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB == NULL)
		win::bug("Could not find function glXCreateContextAttribsARB");

	const int context_attributes[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, options.gl_major,
		GLX_CONTEXT_MINOR_VERSION_ARB, options.gl_minor,
		None
	};

	// create opengl context
	context = glXCreateContextAttribsARB(xdisplay, fbconfig[0], NULL, true, context_attributes);
	if(context == None)
		win::bug("Could not create an OpenGL " + std::to_string(context_attributes[1]) + "." + std::to_string(context_attributes[3])  + " context");
	glXMakeCurrent(xdisplay, window, context);

	// set up delete window protocol
	XSetWMProtocols(xdisplay, window, &atom_delete_window, 1);

	// vsync
	glXSwapIntervalEXT(xdisplay, window, 1);

	XFree(xvi);
	XFree(fbconfig);
}

Display::~Display()
{
	glXMakeCurrent(xdisplay, None, NULL);
	glXDestroyContext(xdisplay, context);
	XDestroyWindow(xdisplay, window);
}

void Display::process()
{
	while(XPending(xdisplay))
	{
		XEvent xevent;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
		case ClientMessage:
			window_handler(win::WindowEvent::CLOSE);
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
				button_handler(Button::MOUSE_LEFT, xevent.type == ButtonPress);
				break;
			case 2:
				button_handler(Button::MOUSE_MIDDLE, xevent.type == ButtonPress);
				break;
			case 3:
				button_handler(Button::MOUSE_RIGHT, xevent.type == ButtonPress);
				break;
			}
			break;
		}
	}
}

void Display::swap()
{
	glXSwapBuffers(xdisplay, window);
}

int Display::width()
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

int Display::height()
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

void Display::cursor(bool show)
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

void Display::vsync(bool on)
{
	glXSwapIntervalEXT(xdisplay, window, on);
}

void Display::register_window_handler(WindowHandler f)
{
	window_handler = std::move(f);
}

void Display::register_button_handler(ButtonHandler f)
{
	button_handler = std::move(f);
}

void Display::register_character_handler(CharacterHandler f)
{
	character_handler = std::move(f);
}

void Display::register_mouse_handler(MouseHandler f)
{
	mouse_handler = std::move(f);
}

int Display::screen_width()
{
	return WidthOfScreen(ScreenOfDisplay(xdisplay, 0));
}

int Display::screen_height()
{
	return HeightOfScreen(ScreenOfDisplay(xdisplay, 0));
}

}

#endif
