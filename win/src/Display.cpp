#include <unordered_map>
#include <string.h>

#include <win.h>

#if defined WINPLAT_LINUX
#include <X11/Xatom.h>
#endif

// default event handlers
static void default_button_handler(win::Button, bool) {}
static void default_character_handler(int) {}
static void default_mouse_handler(int, int) {}

namespace win
{

/* ------------------------------------*/
/////////////////////////////////////////
///// LINUX /////////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/

#if defined WINPLAT_LINUX

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

static Button keystring_to_button(const char *const keystring)
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

Display::Display(const char *caption, int width, int height, bool fullscreen, window_handle parent)
{
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
	window = XCreateWindow(xdisplay, RootWindow(xdisplay, xvi->screen), 0, 0, width, height, 0, xvi->depth, InputOutput, xvi->visual, CWColormap | CWEventMask, &xswa);
	XMapWindow(xdisplay, window);
	XStoreName(xdisplay, window, caption);

	// fullscreen
	if(fullscreen)
		XChangeProperty(xdisplay, window, XInternAtom(xdisplay, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (const unsigned char*)&atom_fullscreen, 1);

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddress((unsigned char*)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB == NULL)
		win::bug("Could not find function glXCreateContextAttribsARB");

	const int context_attributes[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
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

// return false if application is to exit
bool Display::process()
{
	while(XPending(xdisplay))
	{
		XEvent xevent;
		XPeekEvent(xdisplay, &xevent);
		if(xevent.xany.window != window)
			return true;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
		case ClientMessage: return false;

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

	return true;
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

/* ------------------------------------*/
/////////////////////////////////////////
///// WINDOWS ///////////////////////////
/////////////////////////////////////////
/* ------------------------------------*/
#elif defined WINPLAT_WINDOWS

#include <windowsx.h>

static std::unordered_map<unsigned, win::Button> get_physical_keys()
{
	HKL current = GetKeyboardLayout(0);
	HKL qwerty = LoadKeyboardLayout("00000409", KLF_ACTIVATE);

	std::unordered_map<unsigned, win::Button> map;

	map.insert({MapVirtualKeyEx('A', MAPVK_VK_TO_VSC, qwerty), win::Button::A});
	map.insert({MapVirtualKeyEx('B', MAPVK_VK_TO_VSC, qwerty), win::Button::B});
	map.insert({MapVirtualKeyEx('C', MAPVK_VK_TO_VSC, qwerty), win::Button::C});
	map.insert({MapVirtualKeyEx('D', MAPVK_VK_TO_VSC, qwerty), win::Button::D});
	map.insert({MapVirtualKeyEx('E', MAPVK_VK_TO_VSC, qwerty), win::Button::E});
	map.insert({MapVirtualKeyEx('F', MAPVK_VK_TO_VSC, qwerty), win::Button::F});
	map.insert({MapVirtualKeyEx('G', MAPVK_VK_TO_VSC, qwerty), win::Button::G});
	map.insert({MapVirtualKeyEx('H', MAPVK_VK_TO_VSC, qwerty), win::Button::H});
	map.insert({MapVirtualKeyEx('I', MAPVK_VK_TO_VSC, qwerty), win::Button::I});
	map.insert({MapVirtualKeyEx('J', MAPVK_VK_TO_VSC, qwerty), win::Button::J});
	map.insert({MapVirtualKeyEx('K', MAPVK_VK_TO_VSC, qwerty), win::Button::K});
	map.insert({MapVirtualKeyEx('L', MAPVK_VK_TO_VSC, qwerty), win::Button::L});
	map.insert({MapVirtualKeyEx('M', MAPVK_VK_TO_VSC, qwerty), win::Button::M});
	map.insert({MapVirtualKeyEx('N', MAPVK_VK_TO_VSC, qwerty), win::Button::N});
	map.insert({MapVirtualKeyEx('O', MAPVK_VK_TO_VSC, qwerty), win::Button::O});
	map.insert({MapVirtualKeyEx('P', MAPVK_VK_TO_VSC, qwerty), win::Button::P});
	map.insert({MapVirtualKeyEx('Q', MAPVK_VK_TO_VSC, qwerty), win::Button::Q});
	map.insert({MapVirtualKeyEx('R', MAPVK_VK_TO_VSC, qwerty), win::Button::R});
	map.insert({MapVirtualKeyEx('S', MAPVK_VK_TO_VSC, qwerty), win::Button::S});
	map.insert({MapVirtualKeyEx('T', MAPVK_VK_TO_VSC, qwerty), win::Button::T});
	map.insert({MapVirtualKeyEx('U', MAPVK_VK_TO_VSC, qwerty), win::Button::U});
	map.insert({MapVirtualKeyEx('V', MAPVK_VK_TO_VSC, qwerty), win::Button::V});
	map.insert({MapVirtualKeyEx('W', MAPVK_VK_TO_VSC, qwerty), win::Button::W});
	map.insert({MapVirtualKeyEx('X', MAPVK_VK_TO_VSC, qwerty), win::Button::X});
	map.insert({MapVirtualKeyEx('Y', MAPVK_VK_TO_VSC, qwerty), win::Button::Y});
	map.insert({MapVirtualKeyEx('Z', MAPVK_VK_TO_VSC, qwerty), win::Button::Z});

	map.insert({MapVirtualKeyEx(0x30, MAPVK_VK_TO_VSC, qwerty), win::Button::D0});
	map.insert({MapVirtualKeyEx(0x31, MAPVK_VK_TO_VSC, qwerty), win::Button::D1});
	map.insert({MapVirtualKeyEx(0x32, MAPVK_VK_TO_VSC, qwerty), win::Button::D2});
	map.insert({MapVirtualKeyEx(0x33, MAPVK_VK_TO_VSC, qwerty), win::Button::D3});
	map.insert({MapVirtualKeyEx(0x34, MAPVK_VK_TO_VSC, qwerty), win::Button::D4});
	map.insert({MapVirtualKeyEx(0x35, MAPVK_VK_TO_VSC, qwerty), win::Button::D5});
	map.insert({MapVirtualKeyEx(0x36, MAPVK_VK_TO_VSC, qwerty), win::Button::D6});
	map.insert({MapVirtualKeyEx(0x37, MAPVK_VK_TO_VSC, qwerty), win::Button::D7});
	map.insert({MapVirtualKeyEx(0x38, MAPVK_VK_TO_VSC, qwerty), win::Button::D8});
	map.insert({MapVirtualKeyEx(0x39, MAPVK_VK_TO_VSC, qwerty), win::Button::D9});

	map.insert({MapVirtualKeyEx(VK_OEM_3, MAPVK_VK_TO_VSC, qwerty), win::Button::BACKTICK});
	map.insert({MapVirtualKeyEx(VK_OEM_MINUS, MAPVK_VK_TO_VSC, qwerty), win::Button::DASH});
	map.insert({MapVirtualKeyEx(VK_OEM_PLUS, MAPVK_VK_TO_VSC, qwerty), win::Button::EQUALS});
	map.insert({MapVirtualKeyEx(VK_OEM_4, MAPVK_VK_TO_VSC, qwerty), win::Button::LBRACKET});
	map.insert({MapVirtualKeyEx(VK_OEM_6, MAPVK_VK_TO_VSC, qwerty), win::Button::RBRACKET});
	map.insert({MapVirtualKeyEx(VK_OEM_1, MAPVK_VK_TO_VSC, qwerty), win::Button::SEMICOLON});
	map.insert({MapVirtualKeyEx(VK_OEM_7, MAPVK_VK_TO_VSC, qwerty), win::Button::APOSTROPHE});
	map.insert({MapVirtualKeyEx(VK_OEM_COMMA, MAPVK_VK_TO_VSC, qwerty), win::Button::COMMA});
	map.insert({MapVirtualKeyEx(VK_OEM_PERIOD, MAPVK_VK_TO_VSC, qwerty), win::Button::PERIOD});
	map.insert({MapVirtualKeyEx(VK_OEM_2, MAPVK_VK_TO_VSC, qwerty), win::Button::SLASH});
	map.insert({MapVirtualKeyEx(VK_OEM_102, MAPVK_VK_TO_VSC, qwerty), win::Button::BACKSLASH});

	map.insert({MapVirtualKeyEx(VK_F1, MAPVK_VK_TO_VSC, qwerty), win::Button::F1});
	map.insert({MapVirtualKeyEx(VK_F2, MAPVK_VK_TO_VSC, qwerty), win::Button::F2});
	map.insert({MapVirtualKeyEx(VK_F3, MAPVK_VK_TO_VSC, qwerty), win::Button::F3});
	map.insert({MapVirtualKeyEx(VK_F4, MAPVK_VK_TO_VSC, qwerty), win::Button::F4});
	map.insert({MapVirtualKeyEx(VK_F5, MAPVK_VK_TO_VSC, qwerty), win::Button::F5});
	map.insert({MapVirtualKeyEx(VK_F6, MAPVK_VK_TO_VSC, qwerty), win::Button::F6});
	map.insert({MapVirtualKeyEx(VK_F7, MAPVK_VK_TO_VSC, qwerty), win::Button::F7});
	map.insert({MapVirtualKeyEx(VK_F8, MAPVK_VK_TO_VSC, qwerty), win::Button::F8});
	map.insert({MapVirtualKeyEx(VK_F9, MAPVK_VK_TO_VSC, qwerty), win::Button::F9});
	map.insert({MapVirtualKeyEx(VK_F10, MAPVK_VK_TO_VSC, qwerty), win::Button::F10});
	map.insert({MapVirtualKeyEx(VK_F11, MAPVK_VK_TO_VSC, qwerty), win::Button::F11});
	map.insert({MapVirtualKeyEx(VK_F12, MAPVK_VK_TO_VSC, qwerty), win::Button::F12});

	map.insert({MapVirtualKeyEx(VK_ESCAPE, MAPVK_VK_TO_VSC, qwerty), win::Button::ESC});
	// map.insert({MapVirtualKeyEx(VK_SNAPSHOT, MAPVK_VK_TO_VSC, qwerty), win::Button::PRINT_SCR});
	// map.insert({MapVirtualKeyEx(VK_CANCEL, MAPVK_VK_TO_VSC, qwerty), win::Button::PAUSE_BREAK});
	map.insert({MapVirtualKeyEx(VK_INSERT, MAPVK_VK_TO_VSC, qwerty), win::Button::INSERT});
	map.insert({MapVirtualKeyEx(VK_DELETE, MAPVK_VK_TO_VSC, qwerty), win::Button::DELETE});
	map.insert({MapVirtualKeyEx(VK_HOME, MAPVK_VK_TO_VSC, qwerty), win::Button::HOME});
	map.insert({MapVirtualKeyEx(VK_PRIOR, MAPVK_VK_TO_VSC, qwerty), win::Button::PAGE_UP});
	map.insert({MapVirtualKeyEx(VK_NEXT, MAPVK_VK_TO_VSC, qwerty), win::Button::PAGE_DOWN});
	map.insert({MapVirtualKeyEx(VK_END, MAPVK_VK_TO_VSC, qwerty), win::Button::END});
	map.insert({MapVirtualKeyEx(VK_BACK, MAPVK_VK_TO_VSC, qwerty), win::Button::BACKSPACE});
	map.insert({MapVirtualKeyEx(VK_RETURN, MAPVK_VK_TO_VSC, qwerty), win::Button::RETURN});
	// map.insert({MapVirtualKeyEx(VK_EXECUTE, MAPVK_VK_TO_VSC, qwerty), win::Button::ENTER});
	map.insert({MapVirtualKeyEx(VK_LSHIFT, MAPVK_VK_TO_VSC, qwerty), win::Button::LSHIFT});
	map.insert({MapVirtualKeyEx(VK_RSHIFT, MAPVK_VK_TO_VSC, qwerty), win::Button::RSHIFT});
	map.insert({MapVirtualKeyEx(VK_LCONTROL, MAPVK_VK_TO_VSC, qwerty), win::Button::LCTRL});
	map.insert({MapVirtualKeyEx(VK_RCONTROL, MAPVK_VK_TO_VSC, qwerty), win::Button::RCTRL});
	map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::Button::LALT});
	// map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::Button::LALT});
	map.insert({MapVirtualKeyEx(VK_SPACE, MAPVK_VK_TO_VSC, qwerty), win::Button::SPACE});
	// map.insert({MapVirtualKeyEx(VK_RMENU, MAPVK_VK_TO_VSC, qwerty), win::Button::LMETA});
	map.insert({MapVirtualKeyEx(VK_LWIN, MAPVK_VK_TO_VSC, qwerty), win::Button::LMETA});
	map.insert({MapVirtualKeyEx(VK_RWIN, MAPVK_VK_TO_VSC, qwerty), win::Button::RMETA});
	map.insert({MapVirtualKeyEx(VK_UP, MAPVK_VK_TO_VSC, qwerty), win::Button::UP});
	map.insert({MapVirtualKeyEx(VK_DOWN, MAPVK_VK_TO_VSC, qwerty), win::Button::DOWN});
	map.insert({MapVirtualKeyEx(VK_LEFT, MAPVK_VK_TO_VSC, qwerty), win::Button::LEFT});
	map.insert({MapVirtualKeyEx(VK_RIGHT, MAPVK_VK_TO_VSC, qwerty), win::Button::RIGHT});
	map.insert({MapVirtualKeyEx(VK_CAPITAL, MAPVK_VK_TO_VSC, qwerty), win::Button::CAPSLOCK});
	map.insert({MapVirtualKeyEx(VK_TAB, MAPVK_VK_TO_VSC, qwerty), win::Button::TAB});

	map.insert({MapVirtualKeyEx(VK_NUMLOCK, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM_LOCK});
	map.insert({MapVirtualKeyEx(VK_DIVIDE, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM_SLASH});
	map.insert({MapVirtualKeyEx(VK_MULTIPLY, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM_MULTIPLY});
	map.insert({MapVirtualKeyEx(VK_SUBTRACT, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM_MINUS});
	map.insert({MapVirtualKeyEx(VK_ADD, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM_PLUS});
	map.insert({MapVirtualKeyEx(VK_NUMPAD0, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM0});
	map.insert({MapVirtualKeyEx(VK_NUMPAD1, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM1});
	map.insert({MapVirtualKeyEx(VK_NUMPAD2, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM2});
	map.insert({MapVirtualKeyEx(VK_NUMPAD3, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM3});
	map.insert({MapVirtualKeyEx(VK_NUMPAD4, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM4});
	map.insert({MapVirtualKeyEx(VK_NUMPAD5, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM5});
	map.insert({MapVirtualKeyEx(VK_NUMPAD6, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM6});
	map.insert({MapVirtualKeyEx(VK_NUMPAD7, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM7});
	map.insert({MapVirtualKeyEx(VK_NUMPAD8, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM8});
	map.insert({MapVirtualKeyEx(VK_NUMPAD9, MAPVK_VK_TO_VSC, qwerty), win::Button::NUM9});

	ActivateKeyboardLayout(current, KLF_RESET);
	return map;
}

static std::unordered_map<unsigned, win::Button> physical_keys = get_physical_keys();

static win::Button get_physical_key(unsigned scan)
{
	const auto it = physical_keys.find(scan);
	if(it == physical_keys.end())
		return win::Button::UNDEFINED;

	return it->second;
}

void Display::win_init_gl(Display &display, HWND hwnd)
{
	display.hdc = GetDC(hwnd);

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

	SetPixelFormat(display.hdc, ChoosePixelFormat(display.hdc, &pfd), &pfd);
	HGLRC tmp = wglCreateContext(display.hdc);
	wglMakeCurrent(display.hdc, tmp);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (decltype(wglCreateContextAttribsARB))wglGetProcAddress("wglCreateContextAttribsARB");
	display.context = wglCreateContextAttribsARB(display.hdc, NULL, attribs);
	wglMakeCurrent(display.hdc, display.context);
	wglDeleteContext(tmp);
	if(display.context == NULL)
	{
		ReleaseDC(hwnd, display.hdc);
		MessageBox(NULL, "This software requires support for at least Opengl 3.3", "Fatal Error", MB_ICONEXCLAMATION);
		std::abort();
	}
	load_extensions();
}

void Display::win_term_gl()
{
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(context);
	ReleaseDC(window, hdc);
}

LRESULT CALLBACK Display::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT*)lp;
		Display *d = (win::Display*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)d);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		return TRUE;
	}

	Display *const display_ptr = (win::Display*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(display_ptr == NULL)
		return DefWindowProc(hwnd, msg, wp, lp);

	Display &display = *display_ptr;

	switch(msg)
	{
	case WM_CREATE:
		win_init_gl(display, hwnd);
		return 0;
	case WM_CHAR:
		if(wp >= ' ' && wp <= '~')
			display.character_handler(wp);
		return 0;
	case WM_KEYDOWN:
	{
		const unsigned scancode = (lp >> 16) & 0xff;
		const Button key = get_physical_key(scancode);
		if(key == Button::UNDEFINED)
		{
			std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
			return 0;
		}

		display.button_handler(get_physical_key(scancode), true);
		return 0;
	}
	case WM_KEYUP:
	{
		const unsigned scancode = (lp >> 16) & 0xff;
		const Button key = get_physical_key(scancode);
		if(key == Button::UNDEFINED)
		{
			std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
			return 0;
		}

		display.button_handler(get_physical_key(scancode), false);
		return 0;
	}
	case WM_SYSCOMMAND:
		if(wp != SC_KEYMENU)
			return DefWindowProc(hwnd, msg, wp, lp);
	case WM_MOUSEMOVE:
		display.mouse_handler(LOWORD(lp), HIWORD(lp));
		return 0;
	case WM_LBUTTONDOWN:
		display.button_handler(Button::MOUSE_LEFT, true);
		return 0;
	case WM_LBUTTONUP:
		display.button_handler(Button::MOUSE_LEFT, false);
		return 0;
	case WM_RBUTTONDOWN:
		display.button_handler(Button::MOUSE_RIGHT, true);
		return 0;
	case WM_RBUTTONUP:
		display.button_handler(Button::MOUSE_RIGHT, false);
		return 0;
	case WM_MBUTTONDOWN:
		display.button_handler(Button::MOUSE_MIDDLE, true);
		return 0;
	case WM_MBUTTONUP:
		display.button_handler(Button::MOUSE_MIDDLE, false);
		return 0;
	case WM_CLOSE:
		display.winquit = true;
		return 0;
	case WM_ERASEBKGND:
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	win::bug("late return from wndproc");
}

Display::Display(const char *caption, int w, int h, bool fullscreen, window_handle)
{
	const char *const window_class = "win_window_class";

	button_handler = default_button_handler;
	character_handler = default_character_handler;
	mouse_handler = default_mouse_handler;

	winquit = false;

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
		win::bug("Could not register window class");

	if(fullscreen)
		window = CreateWindowEx(0, window_class, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN), NULL, NULL, GetModuleHandle(NULL), this);
	else
		window = CreateWindowEx(0, window_class, caption, WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, GetModuleHandle(NULL), this);
	if(window == NULL)
		win::bug("Could not create window");

	SetWindowText(window, caption);

	ShowWindow(window, SW_SHOWDEFAULT);

	if(!fullscreen)
	{
		RECT rect;
		GetClientRect(window, &rect);
		SetWindowPos(window, NULL, 0, 0, w + (w - rect.right), h + (h - rect.bottom), SWP_SHOWWINDOW);
	}

	glViewport(0, 0, w, h);

	UpdateWindow(window);
}

Display::~Display()
{
	win_term_gl();
	// close the window
	DestroyWindow(window);
}

// return false if application is to exit
bool Display::process()
{
	MSG msg;

	while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return !winquit;
}

void Display::swap()
{
	SwapBuffers(hdc);
}

int Display::width()
{
	RECT rect;
	GetClientRect(window, &rect);

	return rect.right;
}

int Display::height()
{
	RECT rect;
	GetClientRect(window, &rect);

	return rect.bottom;
}

void Display::cursor(bool)
{
}

void Display::vsync(bool on)
{
	wglSwapIntervalEXT(on);
}

void Display::register_button_handler(ButtonHandler fn)
{
	button_handler = std::move(fn);
}

void Display::register_character_handler(CharacterHandler fn)
{
	character_handler = std::move(fn);
}

void Display::register_mouse_handler(MouseHandler fn)
{
	mouse_handler = std::move(fn);
}

int Display::screen_width()
{
	return GetSystemMetrics(SM_CXSCREEN);
}

int Display::screen_height()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

#else
#error "unsupported platform"
#endif
}
