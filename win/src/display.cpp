#include <unordered_map>
#include <string.h>

#include <win.h>

#if defined WINPLAT_LINUX
#include <X11/Xatom.h>
#endif

// default event handlers
static void handler_button(win::button, bool) {}
static void handler_character(int) {}
static void handler_mouse(int, int) {}

win::display::display(display &&rhs)
{
	remote = std::move(rhs.remote);
}

win::display &win::display::operator=(display &&rhs)
{
	finalize();
	remote = std::move(rhs.remote);
	return *this;
}

win::display::~display()
{
	finalize();
}

win::font_renderer win::display::make_font_renderer(int iwidth, int iheight, float left, float right, float bottom, float top) const
{
	return font_renderer(iwidth, iheight, left, right, bottom, top);
}

/* ------------------------------------*/
/////////////////////////////////////////
///// LINUX /////////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

#if defined WINPLAT_LINUX

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static Atom atom_delete_window;
static Atom atom_fullscreen;
static Display *xdisplay;
static XkbDescPtr xkb_desc;
struct x_init_helper
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
			throw win::exception("Could not get the X keyboard map");
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

win::button keystring_to_button(const char *const keystring)
{
	switch(keystring_hash(keystring))
	{
		case keystring_hash("MS1\0"): return win::button::MOUSE_LEFT;
		case keystring_hash("MS2\0"): return win::button::MOUSE_RIGHT;
		case keystring_hash("MS3\0"): return win::button::MOUSE_MIDDLE;
		case keystring_hash("MS4\0"): return win::button::MOUSE4;
		case keystring_hash("MS5\0"): return win::button::MOUSE5;
		case keystring_hash("MS6\0"): return win::button::MOUSE6;
		case keystring_hash("MS7\0"): return win::button::MOUSE7;

		case keystring_hash("AC01"): return win::button::A;
		case keystring_hash("AB05"): return win::button::B;
		case keystring_hash("AB03"): return win::button::C;
		case keystring_hash("AC03"): return win::button::D;
		case keystring_hash("AD03"): return win::button::E;
		case keystring_hash("AC04"): return win::button::F;
		case keystring_hash("AC05"): return win::button::G;
		case keystring_hash("AC06"): return win::button::H;
		case keystring_hash("AD08"): return win::button::I;
		case keystring_hash("AC07"): return win::button::J;
		case keystring_hash("AC08"): return win::button::K;
		case keystring_hash("AC09"): return win::button::L;
		case keystring_hash("AB07"): return win::button::M;
		case keystring_hash("AB06"): return win::button::N;
		case keystring_hash("AD09"): return win::button::O;
		case keystring_hash("AD10"): return win::button::P;
		case keystring_hash("AD01"): return win::button::Q;
		case keystring_hash("AD04"): return win::button::R;
		case keystring_hash("AC02"): return win::button::S;
		case keystring_hash("AD05"): return win::button::T;
		case keystring_hash("AD07"): return win::button::U;
		case keystring_hash("AB04"): return win::button::V;
		case keystring_hash("AD02"): return win::button::W;
		case keystring_hash("AB02"): return win::button::X;
		case keystring_hash("AD06"): return win::button::Y;
		case keystring_hash("AB01"): return win::button::Z;

		case keystring_hash("AE10"): return win::button::D0;
		case keystring_hash("AE01"): return win::button::D1;
		case keystring_hash("AE02"): return win::button::D2;
		case keystring_hash("AE03"): return win::button::D3;
		case keystring_hash("AE04"): return win::button::D4;
		case keystring_hash("AE05"): return win::button::D5;
		case keystring_hash("AE06"): return win::button::D6;
		case keystring_hash("AE07"): return win::button::D7;
		case keystring_hash("AE08"): return win::button::D8;
		case keystring_hash("AE09"): return win::button::D9;

		case keystring_hash("TLDE"): return win::button::BACKTICK;
		case keystring_hash("AE11"): return win::button::DASH;
		case keystring_hash("AE12"): return win::button::EQUALS;
		case keystring_hash("AD11"): return win::button::LBRACKET;
		case keystring_hash("AD12"): return win::button::RBRACKET;
		case keystring_hash("AC10"): return win::button::SEMICOLON;
		case keystring_hash("AC11"): return win::button::APOSTROPHE;
		case keystring_hash("AB08"): return win::button::COMMA;
		case keystring_hash("AB09"): return win::button::PERIOD;
		case keystring_hash("AB10"): return win::button::SLASH;
		case keystring_hash("BKSL"): return win::button::BACKSLASH;

		case keystring_hash("FK01"): return win::button::F1;
		case keystring_hash("FK02"): return win::button::F2;
		case keystring_hash("FK03"): return win::button::F3;
		case keystring_hash("FK04"): return win::button::F4;
		case keystring_hash("FK05"): return win::button::F5;
		case keystring_hash("FK06"): return win::button::F6;
		case keystring_hash("FK07"): return win::button::F7;
		case keystring_hash("FK08"): return win::button::F8;
		case keystring_hash("FK09"): return win::button::F9;
		case keystring_hash("FK10"): return win::button::F10;
		case keystring_hash("FK11"): return win::button::F11;
		case keystring_hash("FK12"): return win::button::F12;

		case keystring_hash("ESC\0"): return win::button::ESC;
		case keystring_hash("PRSC"): return win::button::PRINT_SCR;
		case keystring_hash("PAUS"): return win::button::PAUSE_BREAK;
		case keystring_hash("INS\0"): return win::button::INSERT;
		case keystring_hash("DELE"): return win::button::DELETE;
		case keystring_hash("HOME"): return win::button::HOME;
		case keystring_hash("PGUP"): return win::button::PAGE_UP;
		case keystring_hash("PGDN"): return win::button::PAGE_DOWN;
		case keystring_hash("END\0"): return win::button::END;
		case keystring_hash("BKSP"): return win::button::BACKSPACE;
		case keystring_hash("RTRN"): return win::button::RETURN;
		case keystring_hash("KPEN"): return win::button::ENTER;
		case keystring_hash("LFSH"): return win::button::LSHIFT;
		case keystring_hash("RTSH"): return win::button::RSHIFT;
		case keystring_hash("LCTL"): return win::button::LCTRL;
		case keystring_hash("RCTL"): return win::button::RCTRL;
		case keystring_hash("LALT"): return win::button::LALT;
		case keystring_hash("RALT"): return win::button::RALT;
		case keystring_hash("SPCE"): return win::button::SPACE;
		case keystring_hash("COMP"): return win::button::MENU;
		case keystring_hash("LWIN"): return win::button::LMETA;
		case keystring_hash("RWIN"): return win::button::RMETA;

		case keystring_hash("UP\0\0"): return win::button::UP;
		case keystring_hash("LEFT"): return win::button::LEFT;
		case keystring_hash("RGHT"): return win::button::RIGHT;
		case keystring_hash("DOWN"): return win::button::DOWN;

		case keystring_hash("CAPS"): return win::button::CAPSLOCK;
		case keystring_hash("TAB\0"): return win::button::TAB;
		case keystring_hash("NMLK"): return win::button::NUM_LOCK;
		case keystring_hash("KPDV"): return win::button::NUM_SLASH;
		case keystring_hash("KPMU"): return win::button::NUM_MULTIPLY;
		case keystring_hash("KPSU"): return win::button::NUM_MINUS;
		case keystring_hash("KPAD"): return win::button::NUM_PLUS;
		case keystring_hash("KPDL"): return win::button::NUM_DEL;

		case keystring_hash("KP0\0"): return win::button::NUM0;
		case keystring_hash("KP1\0"): return win::button::NUM1;
		case keystring_hash("KP2\0"): return win::button::NUM2;
		case keystring_hash("KP3\0"): return win::button::NUM3;
		case keystring_hash("KP4\0"): return win::button::NUM4;
		case keystring_hash("KP5\0"): return win::button::NUM5;
		case keystring_hash("KP6\0"): return win::button::NUM6;
		case keystring_hash("KP7\0"): return win::button::NUM7;
		case keystring_hash("KP8\0"): return win::button::NUM8;
		case keystring_hash("KP9\0"): return win::button::NUM9;

		default: return win::button::UNDEFINED;
	}
}

win::display::display(const char *caption, int width, int height, int flags, window_handle parent)
{
	remote.reset(new display_remote);

	remote->handler.key_button = handler_button;
	remote->handler.character = handler_character;
	remote->handler.mouse = handler_mouse;
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
	remote->window_ = XCreateWindow(xdisplay, RootWindow(xdisplay, xvi->screen), 0, 0, width, height, 0, xvi->depth, InputOutput, xvi->visual, CWColormap | CWEventMask, &xswa);
	XMapWindow(xdisplay, remote->window_);
	XStoreName(xdisplay, remote->window_, caption);

	// fullscreen
	if(flags & FULLSCREEN)
		XChangeProperty(xdisplay, remote->window_, XInternAtom(xdisplay, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (const unsigned char*)&atom_fullscreen, 1);

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
	remote->context_ = glXCreateContextAttribsARB(xdisplay, fbconfig[0], NULL, true, context_attributes);
	if(remote->context_ == None)
		throw exception("Could not create an OpenGL " + std::to_string(context_attributes[1]) + "." + std::to_string(context_attributes[3])  + " context");
	glXMakeCurrent(xdisplay, remote->window_, remote->context_);

	// set up delete window protocol
	XSetWMProtocols(xdisplay, remote->window_, &atom_delete_window, 1);

	// vsync
	glXSwapIntervalEXT(xdisplay, remote->window_, 1);

	XFree(xvi);
	XFree(fbconfig);
}

// return false if application is to exit
bool win::display::process()
{
	// handle evdev event-joystick
	process_joystick();

	while(XPending(xdisplay))
	{
		XEvent xevent;
		XPeekEvent(xdisplay, &xevent);
		if(xevent.xany.window != remote->window_)
			return true;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
			case ClientMessage: return false;

			case KeyPress:
			{
				remote->handler.key_button(keystring_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), true);
				const KeySym sym = x_get_keysym(&xevent.xkey);
				if(sym)
					remote->handler.character(sym);
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

				remote->handler.key_button(keystring_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), false);
				break;
			}
			case MotionNotify:
				remote->handler.mouse(xevent.xmotion.x, xevent.xmotion.y);
				break;
			case ButtonPress:
			case ButtonRelease:
				switch(xevent.xbutton.button)
				{
					case 1:
						remote->handler.key_button(button::MOUSE_LEFT, xevent.type == ButtonPress);
						break;
					case 2:
						remote->handler.key_button(button::MOUSE_MIDDLE, xevent.type == ButtonPress);
						break;
					case 3:
						remote->handler.key_button(button::MOUSE_RIGHT, xevent.type == ButtonPress);
						break;
				}
				break;
		}
	}

	return true;
}

void win::display::swap() const
{
	glXSwapBuffers(xdisplay, remote->window_);
}

int win::display::width() const
{
	Window root;
	int xpos;
	int ypos;
	unsigned width = 0;
	unsigned height;
	unsigned border;
	unsigned depth;

	XGetGeometry(xdisplay, remote->window_, &root, &xpos, &ypos, &width, &height, &border, &depth);

	return width;
}

int win::display::height() const
{
	Window root;
	int xpos;
	int ypos;
	unsigned width;
	unsigned height = 0;
	unsigned border;
	unsigned depth;

	XGetGeometry(xdisplay, remote->window_, &root, &xpos, &ypos, &width, &height, &border, &depth);

	return height;
}

void win::display::cursor(bool show)
{
	if(show)
	{
		XUndefineCursor(xdisplay, remote->window_);
	}
	else
	{
		Pixmap pm;
		XColor dummy;
		char data[2] = {0, 0};
		Cursor cursor;

		pm = XCreateBitmapFromData(xdisplay, remote->window_, data, 1, 1);
		cursor = XCreatePixmapCursor(xdisplay, pm, pm, &dummy, &dummy, 0, 0);
		XFreePixmap(xdisplay, pm);

		XDefineCursor(xdisplay, remote->window_, cursor);
	}
}

void win::display::vsync(bool on)
{
	glXSwapIntervalEXT(xdisplay, remote->window_, on);
}

void win::display::event_button(fn_event_button f)
{
	remote->handler.key_button = f;
	remote->joystick_.event_button(f);
}

void win::display::event_joystick(fn_event_joystick f)
{
	remote->joystick_.event_joystick(std::move(f));
}

void win::display::event_character(fn_event_character f)
{
	remote->handler.character = std::move(f);
}

void win::display::event_mouse(fn_event_mouse f)
{
	remote->handler.mouse = std::move(f);
}

int win::display::screen_width()
{
	return WidthOfScreen(ScreenOfDisplay(xdisplay, 0));
}

int win::display::screen_height()
{
	return HeightOfScreen(ScreenOfDisplay(xdisplay, 0));
}

win::audio_engine win::display::make_audio_engine(audio_engine::sound_config_fn fn)
{
	return audio_engine(fn);
}

void win::display::process_joystick()
{
	remote->joystick_.process();
}

void win::display::finalize()
{
	if(!remote)
		return;

	glXMakeCurrent(xdisplay, None, NULL);
	glXDestroyContext(xdisplay, remote->context_);
	XDestroyWindow(xdisplay, remote->window_);

	remote.reset();
}

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/
#elif defined WINPLAT_WINDOWS

#include <windowsx.h>

static std::unordered_map<unsigned, win::button> get_physical_keys()
{
	HKL current = GetKeyboardLayout(0);
	HKL qwerty = LoadKeyboardLayout("00000409", KLF_ACTIVATE);

	std::unordered_map<unsigned, win::button> map;

	map.insert({MapVirtualKeyEx('A', MAPVK_VK_TO_VSC, qwerty), win::button::A});
	map.insert({MapVirtualKeyEx('B', MAPVK_VK_TO_VSC, qwerty), win::button::B});
	map.insert({MapVirtualKeyEx('C', MAPVK_VK_TO_VSC, qwerty), win::button::C});
	map.insert({MapVirtualKeyEx('D', MAPVK_VK_TO_VSC, qwerty), win::button::D});
	map.insert({MapVirtualKeyEx('E', MAPVK_VK_TO_VSC, qwerty), win::button::E});
	map.insert({MapVirtualKeyEx('F', MAPVK_VK_TO_VSC, qwerty), win::button::F});
	map.insert({MapVirtualKeyEx('G', MAPVK_VK_TO_VSC, qwerty), win::button::G});
	map.insert({MapVirtualKeyEx('H', MAPVK_VK_TO_VSC, qwerty), win::button::H});
	map.insert({MapVirtualKeyEx('I', MAPVK_VK_TO_VSC, qwerty), win::button::I});
	map.insert({MapVirtualKeyEx('J', MAPVK_VK_TO_VSC, qwerty), win::button::J});
	map.insert({MapVirtualKeyEx('K', MAPVK_VK_TO_VSC, qwerty), win::button::K});
	map.insert({MapVirtualKeyEx('L', MAPVK_VK_TO_VSC, qwerty), win::button::L});
	map.insert({MapVirtualKeyEx('M', MAPVK_VK_TO_VSC, qwerty), win::button::M});
	map.insert({MapVirtualKeyEx('N', MAPVK_VK_TO_VSC, qwerty), win::button::N});
	map.insert({MapVirtualKeyEx('O', MAPVK_VK_TO_VSC, qwerty), win::button::O});
	map.insert({MapVirtualKeyEx('P', MAPVK_VK_TO_VSC, qwerty), win::button::P});
	map.insert({MapVirtualKeyEx('Q', MAPVK_VK_TO_VSC, qwerty), win::button::Q});
	map.insert({MapVirtualKeyEx('R', MAPVK_VK_TO_VSC, qwerty), win::button::R});
	map.insert({MapVirtualKeyEx('S', MAPVK_VK_TO_VSC, qwerty), win::button::S});
	map.insert({MapVirtualKeyEx('T', MAPVK_VK_TO_VSC, qwerty), win::button::T});
	map.insert({MapVirtualKeyEx('U', MAPVK_VK_TO_VSC, qwerty), win::button::U});
	map.insert({MapVirtualKeyEx('V', MAPVK_VK_TO_VSC, qwerty), win::button::V});
	map.insert({MapVirtualKeyEx('W', MAPVK_VK_TO_VSC, qwerty), win::button::W});
	map.insert({MapVirtualKeyEx('X', MAPVK_VK_TO_VSC, qwerty), win::button::X});
	map.insert({MapVirtualKeyEx('Y', MAPVK_VK_TO_VSC, qwerty), win::button::Y});
	map.insert({MapVirtualKeyEx('Z', MAPVK_VK_TO_VSC, qwerty), win::button::Z});

	map.insert({MapVirtualKeyEx(0x30, MAPVK_VK_TO_VSC, qwerty), win::button::D0});
	map.insert({MapVirtualKeyEx(0x31, MAPVK_VK_TO_VSC, qwerty), win::button::D1});
	map.insert({MapVirtualKeyEx(0x32, MAPVK_VK_TO_VSC, qwerty), win::button::D2});
	map.insert({MapVirtualKeyEx(0x33, MAPVK_VK_TO_VSC, qwerty), win::button::D3});
	map.insert({MapVirtualKeyEx(0x34, MAPVK_VK_TO_VSC, qwerty), win::button::D4});
	map.insert({MapVirtualKeyEx(0x35, MAPVK_VK_TO_VSC, qwerty), win::button::D5});
	map.insert({MapVirtualKeyEx(0x36, MAPVK_VK_TO_VSC, qwerty), win::button::D6});
	map.insert({MapVirtualKeyEx(0x37, MAPVK_VK_TO_VSC, qwerty), win::button::D7});
	map.insert({MapVirtualKeyEx(0x38, MAPVK_VK_TO_VSC, qwerty), win::button::D8});
	map.insert({MapVirtualKeyEx(0x39, MAPVK_VK_TO_VSC, qwerty), win::button::D9});

	map.insert({MapVirtualKeyEx(VK_OEM_3, MAPVK_VK_TO_VSC, qwerty), win::button::BACKTICK});
	map.insert({MapVirtualKeyEx(VK_OEM_MINUS, MAPVK_VK_TO_VSC, qwerty), win::button::DASH});
	map.insert({MapVirtualKeyEx(VK_OEM_PLUS, MAPVK_VK_TO_VSC, qwerty), win::button::EQUALS});
	map.insert({MapVirtualKeyEx(VK_OEM_4, MAPVK_VK_TO_VSC, qwerty), win::button::LBRACKET});
	map.insert({MapVirtualKeyEx(VK_OEM_6, MAPVK_VK_TO_VSC, qwerty), win::button::RBRACKET});
	map.insert({MapVirtualKeyEx(VK_OEM_1, MAPVK_VK_TO_VSC, qwerty), win::button::SEMICOLON});
	map.insert({MapVirtualKeyEx(VK_OEM_7, MAPVK_VK_TO_VSC, qwerty), win::button::APOSTROPHE});
	map.insert({MapVirtualKeyEx(VK_OEM_COMMA, MAPVK_VK_TO_VSC, qwerty), win::button::COMMA});
	map.insert({MapVirtualKeyEx(VK_OEM_PERIOD, MAPVK_VK_TO_VSC, qwerty), win::button::PERIOD});
	map.insert({MapVirtualKeyEx(VK_OEM_2, MAPVK_VK_TO_VSC, qwerty), win::button::SLASH});
	map.insert({MapVirtualKeyEx(VK_OEM_102, MAPVK_VK_TO_VSC, qwerty), win::button::BACKSLASH});

	map.insert({MapVirtualKeyEx(VK_F1, MAPVK_VK_TO_VSC, qwerty), win::button::F1});
	map.insert({MapVirtualKeyEx(VK_F2, MAPVK_VK_TO_VSC, qwerty), win::button::F2});
	map.insert({MapVirtualKeyEx(VK_F3, MAPVK_VK_TO_VSC, qwerty), win::button::F3});
	map.insert({MapVirtualKeyEx(VK_F4, MAPVK_VK_TO_VSC, qwerty), win::button::F4});
	map.insert({MapVirtualKeyEx(VK_F5, MAPVK_VK_TO_VSC, qwerty), win::button::F5});
	map.insert({MapVirtualKeyEx(VK_F6, MAPVK_VK_TO_VSC, qwerty), win::button::F6});
	map.insert({MapVirtualKeyEx(VK_F7, MAPVK_VK_TO_VSC, qwerty), win::button::F7});
	map.insert({MapVirtualKeyEx(VK_F8, MAPVK_VK_TO_VSC, qwerty), win::button::F8});
	map.insert({MapVirtualKeyEx(VK_F9, MAPVK_VK_TO_VSC, qwerty), win::button::F9});
	map.insert({MapVirtualKeyEx(VK_F10, MAPVK_VK_TO_VSC, qwerty), win::button::F10});
	map.insert({MapVirtualKeyEx(VK_F11, MAPVK_VK_TO_VSC, qwerty), win::button::F11});
	map.insert({MapVirtualKeyEx(VK_F12, MAPVK_VK_TO_VSC, qwerty), win::button::F12});

	map.insert({MapVirtualKeyEx(VK_ESCAPE, MAPVK_VK_TO_VSC, qwerty), win::button::ESC});
	// map.insert({MapVirtualKeyEx(VK_SNAPSHOT, MAPVK_VK_TO_VSC, qwerty), win::button::PRINT_SCR});
	// map.insert({MapVirtualKeyEx(VK_CANCEL, MAPVK_VK_TO_VSC, qwerty), win::button::PAUSE_BREAK});
	map.insert({MapVirtualKeyEx(VK_INSERT, MAPVK_VK_TO_VSC, qwerty), win::button::INSERT});
	map.insert({MapVirtualKeyEx(VK_DELETE, MAPVK_VK_TO_VSC, qwerty), win::button::DELETE});
	map.insert({MapVirtualKeyEx(VK_HOME, MAPVK_VK_TO_VSC, qwerty), win::button::HOME});
	map.insert({MapVirtualKeyEx(VK_PRIOR, MAPVK_VK_TO_VSC, qwerty), win::button::PAGE_UP});
	map.insert({MapVirtualKeyEx(VK_NEXT, MAPVK_VK_TO_VSC, qwerty), win::button::PAGE_DOWN});
	map.insert({MapVirtualKeyEx(VK_END, MAPVK_VK_TO_VSC, qwerty), win::button::END});
	map.insert({MapVirtualKeyEx(VK_BACK, MAPVK_VK_TO_VSC, qwerty), win::button::BACKSPACE});
	map.insert({MapVirtualKeyEx(VK_RETURN, MAPVK_VK_TO_VSC, qwerty), win::button::RETURN});
	// map.insert({MapVirtualKeyEx(VK_EXECUTE, MAPVK_VK_TO_VSC, qwerty), win::button::ENTER});
	map.insert({MapVirtualKeyEx(VK_LSHIFT, MAPVK_VK_TO_VSC, qwerty), win::button::LSHIFT});
	map.insert({MapVirtualKeyEx(VK_RSHIFT, MAPVK_VK_TO_VSC, qwerty), win::button::RSHIFT});
	map.insert({MapVirtualKeyEx(VK_LCONTROL, MAPVK_VK_TO_VSC, qwerty), win::button::LCTRL});
	map.insert({MapVirtualKeyEx(VK_RCONTROL, MAPVK_VK_TO_VSC, qwerty), win::button::RCTRL});
	map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::button::LALT});
	// map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::button::LALT});
	map.insert({MapVirtualKeyEx(VK_SPACE, MAPVK_VK_TO_VSC, qwerty), win::button::SPACE});
	// map.insert({MapVirtualKeyEx(VK_RMENU, MAPVK_VK_TO_VSC, qwerty), win::button::LMETA});
	map.insert({MapVirtualKeyEx(VK_LWIN, MAPVK_VK_TO_VSC, qwerty), win::button::LMETA});
	map.insert({MapVirtualKeyEx(VK_RWIN, MAPVK_VK_TO_VSC, qwerty), win::button::RMETA});
	map.insert({MapVirtualKeyEx(VK_UP, MAPVK_VK_TO_VSC, qwerty), win::button::UP});
	map.insert({MapVirtualKeyEx(VK_DOWN, MAPVK_VK_TO_VSC, qwerty), win::button::DOWN});
	map.insert({MapVirtualKeyEx(VK_LEFT, MAPVK_VK_TO_VSC, qwerty), win::button::LEFT});
	map.insert({MapVirtualKeyEx(VK_RIGHT, MAPVK_VK_TO_VSC, qwerty), win::button::RIGHT});
	map.insert({MapVirtualKeyEx(VK_CAPITAL, MAPVK_VK_TO_VSC, qwerty), win::button::CAPSLOCK});
	map.insert({MapVirtualKeyEx(VK_TAB, MAPVK_VK_TO_VSC, qwerty), win::button::TAB});

	map.insert({MapVirtualKeyEx(VK_NUMLOCK, MAPVK_VK_TO_VSC, qwerty), win::button::NUM_LOCK});
	map.insert({MapVirtualKeyEx(VK_DIVIDE, MAPVK_VK_TO_VSC, qwerty), win::button::NUM_SLASH});
	map.insert({MapVirtualKeyEx(VK_MULTIPLY, MAPVK_VK_TO_VSC, qwerty), win::button::NUM_MULTIPLY});
	map.insert({MapVirtualKeyEx(VK_SUBTRACT, MAPVK_VK_TO_VSC, qwerty), win::button::NUM_MINUS});
	map.insert({MapVirtualKeyEx(VK_ADD, MAPVK_VK_TO_VSC, qwerty), win::button::NUM_PLUS});
	map.insert({MapVirtualKeyEx(VK_NUMPAD0, MAPVK_VK_TO_VSC, qwerty), win::button::NUM0});
	map.insert({MapVirtualKeyEx(VK_NUMPAD1, MAPVK_VK_TO_VSC, qwerty), win::button::NUM1});
	map.insert({MapVirtualKeyEx(VK_NUMPAD2, MAPVK_VK_TO_VSC, qwerty), win::button::NUM2});
	map.insert({MapVirtualKeyEx(VK_NUMPAD3, MAPVK_VK_TO_VSC, qwerty), win::button::NUM3});
	map.insert({MapVirtualKeyEx(VK_NUMPAD4, MAPVK_VK_TO_VSC, qwerty), win::button::NUM4});
	map.insert({MapVirtualKeyEx(VK_NUMPAD5, MAPVK_VK_TO_VSC, qwerty), win::button::NUM5});
	map.insert({MapVirtualKeyEx(VK_NUMPAD6, MAPVK_VK_TO_VSC, qwerty), win::button::NUM6});
	map.insert({MapVirtualKeyEx(VK_NUMPAD7, MAPVK_VK_TO_VSC, qwerty), win::button::NUM7});
	map.insert({MapVirtualKeyEx(VK_NUMPAD8, MAPVK_VK_TO_VSC, qwerty), win::button::NUM8});
	map.insert({MapVirtualKeyEx(VK_NUMPAD9, MAPVK_VK_TO_VSC, qwerty), win::button::NUM9});

	ActivateKeyboardLayout(current, KLF_RESET);
	return map;
}

static std::unordered_map<unsigned, win::button> physical_keys = get_physical_keys();

static win::button get_physical_key(unsigned scan)
{
	const auto it = physical_keys.find(scan);
	if(it == physical_keys.end())
		return win::button::UNDEFINED;

	return it->second;
}

void win::display::win_init_gl(display_remote *remote, HWND hwnd)
{
	remote->hdc_ = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits=24;
	pfd.cStencilBits=8;
	pfd.iLayerType=PFD_MAIN_PLANE;

	const int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, 0
	};

	SetPixelFormat(remote->hdc_, ChoosePixelFormat(remote->hdc_, &pfd), &pfd);
	HGLRC tmp = wglCreateContext(remote->hdc_);
	wglMakeCurrent(remote->hdc_, tmp);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (decltype(wglCreateContextAttribsARB))wglGetProcAddress("wglCreateContextAttribsARB");
	remote->context_ = wglCreateContextAttribsARB(remote->hdc_, NULL, attribs);
	wglMakeCurrent(remote->hdc_, remote->context_);
	wglDeleteContext(tmp);
	if(remote->context_ == NULL)
	{
		ReleaseDC(hwnd, remote->hdc_);
		MessageBox(NULL, "This software requires support for at least Opengl 3.3", "Fatal Error", MB_ICONEXCLAMATION);
		std::abort();
	}
	load_extensions();
}

void win::display::win_term_gl()
{
	wglMakeCurrent(remote->hdc_, NULL);
	wglDeleteContext(remote->context_);
	ReleaseDC(remote->window_, remote->hdc_);
}

LRESULT CALLBACK win::display::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT*)lp;
		win::display_remote *d = (win::display_remote*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)d);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		return TRUE;
	}

	win::display_remote *const remote = (win::display_remote*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(remote == NULL)
		return DefWindowProc(hwnd, msg, wp, lp);


	switch(msg)
	{
		case WM_CREATE:
			win_init_gl(remote, hwnd);
			return 0;
		case WM_CHAR:
			if(wp >= ' ' && wp <= '~')
				remote->handler.character(wp);
			return 0;
		case WM_KEYDOWN:
		{
			const unsigned scancode = (lp >> 16) & 0xff;
			const win::button key = get_physical_key(scancode);
			if(key == win::button::UNDEFINED)
			{
				std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
				return 0;
			}

			remote->handler.key_button(get_physical_key(scancode), true);
			return 0;
		}
		case WM_KEYUP:
		{
			const unsigned scancode = (lp >> 16) & 0xff;
			const win::button key = get_physical_key(scancode);
			if(key == win::button::UNDEFINED)
			{
				std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
				return 0;
			}

			remote->handler.key_button(get_physical_key(scancode), false);
			return 0;
		}
		case WM_SYSCOMMAND:
			if(wp != SC_KEYMENU)
				return DefWindowProc(hwnd, msg, wp, lp);
		case WM_MOUSEMOVE:
			remote->handler.mouse(LOWORD(lp), HIWORD(lp));
			return 0;
		case WM_LBUTTONDOWN:
			remote->handler.key_button(button::MOUSE_LEFT, true);
			return 0;
		case WM_LBUTTONUP:
			remote->handler.key_button(button::MOUSE_LEFT, false);
			return 0;
		case WM_RBUTTONDOWN:
			remote->handler.key_button(button::MOUSE_RIGHT, true);
			return 0;
		case WM_RBUTTONUP:
			remote->handler.key_button(button::MOUSE_RIGHT, false);
			return 0;
		case WM_MBUTTONDOWN:
			remote->handler.key_button(button::MOUSE_MIDDLE, true);
			return 0;
		case WM_MBUTTONUP:
			remote->handler.key_button(button::MOUSE_MIDDLE, false);
			return 0;
		case WM_CLOSE:
			remote->winquit_ = true;
			return 0;
		case WM_ERASEBKGND:
			return 0;
		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}

	win::bug("late return from wndproc");
}

win::display::display(const char *caption, int w, int h, int flags, window_handle)
{
	const char *const window_class = "win_window_class";

	remote.reset(new display_remote);

	remote->handler.key_button = handler_button;
	remote->handler.character = handler_character;
	remote->handler.mouse = handler_mouse;
	remote->directsound_ = NULL;

	remote->winquit_ = false;

	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = window_class;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
		throw exception("Could not register window class");

	if(flags & FULLSCREEN)
		remote->window_ = CreateWindowEx(0, window_class, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN), NULL, NULL, GetModuleHandle(NULL), remote.get());
	else
		remote->window_ = CreateWindowEx(0, window_class, caption, WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, GetModuleHandle(NULL), remote.get());
	if(remote->window_ == NULL)
		throw exception("Could not create window");

	SetWindowText(remote->window_, caption);

	ShowWindow(remote->window_, SW_SHOWDEFAULT);

	const int result = flags & FULLSCREEN;
	if((flags & FULLSCREEN) == 0)
	{
		RECT rect;
		GetClientRect(remote->window_, &rect);
		SetWindowPos(remote->window_, NULL, 0, 0, w + (w - rect.right), h + (h - rect.bottom), SWP_SHOWWINDOW);
	}

	glViewport(0, 0, w, h);

	UpdateWindow(remote->window_);
}

// return false if application is to exit
bool win::display::process()
{
	MSG msg;

	while(PeekMessage(&msg, remote->window_, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// poke the directsound system
	if(remote->directsound_ != NULL)
		win::audio_engine::poke(remote->directsound_);

	return !remote->winquit_;
}

void win::display::swap() const
{
	SwapBuffers(remote->hdc_);
}

int win::display::width() const
{
	RECT rect;
	GetClientRect(remote->window_, &rect);

	return rect.right;
}

int win::display::height() const
{
	RECT rect;
	GetClientRect(remote->window_, &rect);

	return rect.bottom;
}

void win::display::cursor(bool)
{
}

void win::display::vsync(bool on)
{
	wglSwapIntervalEXT(on);
}

void win::display::event_button(fn_event_button fn)
{
	remote->handler.key_button = std::move(fn);
}

void win::display::event_joystick(fn_event_joystick)
{
}

void win::display::event_character(fn_event_character fn)
{
	remote->handler.character = std::move(fn);
}

void win::display::event_mouse(fn_event_mouse fn)
{
	remote->handler.mouse = std::move(fn);
}

int win::display::screen_width()
{
	return GetSystemMetrics(SM_CXSCREEN);
}

int win::display::screen_height()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

win::audio_engine win::display::make_audio_engine(audio_engine::sound_config_fn fn)
{
	return audio_engine(fn, this);
}

void win::display::process_joystick()
{
}

void win::display::finalize()
{
	if(!remote)
		return;

	win_term_gl();
	// close the window
	DestroyWindow(remote->window_);

	remote.reset();
}

#else
#error "unsupported platform"
#endif
