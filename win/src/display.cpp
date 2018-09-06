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

win::display::display()
{
#if defined WINPLAT_LINUX
	window_ = None;
#elif defined WINPLAT_WINDOWS
	window_ = NULL;
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

win::audio_engine win::display::make_audio_engine(audio_engine::sound_config_fn fn) const
{
	return audio_engine(fn);
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

static std::unordered_map<std::string, win::button> load_physical_keys()
{
	std::unordered_map<std::string, win::button> map;

	map.insert({name_to_string("MS1\0"), win::button::MOUSE_LEFT});
	map.insert({name_to_string("MS2\0"), win::button::MOUSE_RIGHT});
	map.insert({name_to_string("MS3\0"), win::button::MOUSE_MIDDLE});
	map.insert({name_to_string("MS4\0"), win::button::MOUSE4});
	map.insert({name_to_string("MS5\0"), win::button::MOUSE5});
	map.insert({name_to_string("MS6\0"), win::button::MOUSE6});
	map.insert({name_to_string("MS7\0"), win::button::MOUSE7});

	map.insert({name_to_string("AC01"), win::button::A});
	map.insert({name_to_string("AB05"), win::button::B});
	map.insert({name_to_string("AB03"), win::button::C});
	map.insert({name_to_string("AC03"), win::button::D});
	map.insert({name_to_string("AD03"), win::button::E});
	map.insert({name_to_string("AC04"), win::button::F});
	map.insert({name_to_string("AC05"), win::button::G});
	map.insert({name_to_string("AC06"), win::button::H});
	map.insert({name_to_string("AD08"), win::button::I});
	map.insert({name_to_string("AC07"), win::button::J});
	map.insert({name_to_string("AC08"), win::button::K});
	map.insert({name_to_string("AC09"), win::button::L});
	map.insert({name_to_string("AB07"), win::button::M});
	map.insert({name_to_string("AB06"), win::button::N});
	map.insert({name_to_string("AD09"), win::button::O});
	map.insert({name_to_string("AD10"), win::button::P});
	map.insert({name_to_string("AD01"), win::button::Q});
	map.insert({name_to_string("AD04"), win::button::R});
	map.insert({name_to_string("AC02"), win::button::S});
	map.insert({name_to_string("AD05"), win::button::T});
	map.insert({name_to_string("AD07"), win::button::U});
	map.insert({name_to_string("AB04"), win::button::V});
	map.insert({name_to_string("AD02"), win::button::W});
	map.insert({name_to_string("AB02"), win::button::X});
	map.insert({name_to_string("AD06"), win::button::Y});
	map.insert({name_to_string("AB01"), win::button::Z});

	map.insert({name_to_string("AE10"), win::button::D0});
	map.insert({name_to_string("AE01"), win::button::D1});
	map.insert({name_to_string("AE02"), win::button::D2});
	map.insert({name_to_string("AE03"), win::button::D3});
	map.insert({name_to_string("AE04"), win::button::D4});
	map.insert({name_to_string("AE05"), win::button::D5});
	map.insert({name_to_string("AE06"), win::button::D6});
	map.insert({name_to_string("AE07"), win::button::D7});
	map.insert({name_to_string("AE08"), win::button::D8});
	map.insert({name_to_string("AE09"), win::button::D9});

	map.insert({name_to_string("TLDE"), win::button::TILDE});
	map.insert({name_to_string("AE11"), win::button::DASH});
	map.insert({name_to_string("AE12"), win::button::EQUALS});
	map.insert({name_to_string("AD11"), win::button::LBRACKET});
	map.insert({name_to_string("AD12"), win::button::RBRACKET});
	map.insert({name_to_string("AC10"), win::button::SEMICOLON});
	map.insert({name_to_string("AC11"), win::button::APOSTROPHE});
	map.insert({name_to_string("AB08"), win::button::COMMA});
	map.insert({name_to_string("AB09"), win::button::PERIOD});
	map.insert({name_to_string("AB10"), win::button::SLASH});
	map.insert({name_to_string("BKSL"), win::button::BACKSLASH});

	map.insert({name_to_string("FK01"), win::button::F1});
	map.insert({name_to_string("FK02"), win::button::F2});
	map.insert({name_to_string("FK03"), win::button::F3});
	map.insert({name_to_string("FK04"), win::button::F4});
	map.insert({name_to_string("FK05"), win::button::F5});
	map.insert({name_to_string("FK06"), win::button::F6});
	map.insert({name_to_string("FK07"), win::button::F7});
	map.insert({name_to_string("FK08"), win::button::F8});
	map.insert({name_to_string("FK09"), win::button::F9});
	map.insert({name_to_string("FK10"), win::button::F10});
	map.insert({name_to_string("FK11"), win::button::F11});
	map.insert({name_to_string("FK12"), win::button::F12});

	map.insert({name_to_string("ESC\0"), win::button::ESC});
	map.insert({name_to_string("PRSC"), win::button::PRINT_SCR});
	map.insert({name_to_string("PAUS"), win::button::PAUSE_BREAK});
	map.insert({name_to_string("INS\0"), win::button::INSERT});
	map.insert({name_to_string("DELE"), win::button::DELETE});
	map.insert({name_to_string("HOME"), win::button::HOME});
	map.insert({name_to_string("PGUP"), win::button::PAGE_UP});
	map.insert({name_to_string("PGDN"), win::button::PAGE_DOWN});
	map.insert({name_to_string("END\0"), win::button::END});
	map.insert({name_to_string("BKSP"), win::button::BACKSPACE});
	map.insert({name_to_string("RTRN"), win::button::RETURN});
	map.insert({name_to_string("KPEN"), win::button::ENTER});
	map.insert({name_to_string("LFSH"), win::button::LSHIFT});
	map.insert({name_to_string("RTSH"), win::button::RSHIFT});
	map.insert({name_to_string("LCTL"), win::button::LCTRL});
	map.insert({name_to_string("RCTL"), win::button::RCTRL});
	map.insert({name_to_string("LALT"), win::button::LALT});
	map.insert({name_to_string("RALT"), win::button::RALT});
	map.insert({name_to_string("SPCE"), win::button::SPACE});
	map.insert({name_to_string("COMP"), win::button::MENU});
	map.insert({name_to_string("LWIN"), win::button::LMETA});
	map.insert({name_to_string("RWIN"), win::button::RMETA});
	map.insert({name_to_string("UP\0\0"), win::button::UP});
	map.insert({name_to_string("LEFT"), win::button::LEFT});
	map.insert({name_to_string("RGHT"), win::button::RIGHT});
	map.insert({name_to_string("DOWN"), win::button::DOWN});
	map.insert({name_to_string("CAPS"), win::button::CAPSLOCK});
	map.insert({name_to_string("TAB\0"), win::button::TAB});

	map.insert({name_to_string("NMLK"), win::button::NUM_LOCK});
	map.insert({name_to_string("KPDV"), win::button::NUM_SLASH});
	map.insert({name_to_string("KPMU"), win::button::NUM_MULTIPLY});
	map.insert({name_to_string("KPSU"), win::button::NUM_MINUS});
	map.insert({name_to_string("KPAD"), win::button::NUM_PLUS});
	map.insert({name_to_string("KPDL"), win::button::NUM_DEL});
	map.insert({name_to_string("KP0\0"), win::button::NUM0});
	map.insert({name_to_string("KP1\0"), win::button::NUM1});
	map.insert({name_to_string("KP2\0"), win::button::NUM2});
	map.insert({name_to_string("KP3\0"), win::button::NUM3});
	map.insert({name_to_string("KP4\0"), win::button::NUM4});
	map.insert({name_to_string("KP5\0"), win::button::NUM5});
	map.insert({name_to_string("KP6\0"), win::button::NUM6});
	map.insert({name_to_string("KP7\0"), win::button::NUM7});
	map.insert({name_to_string("KP8\0"), win::button::NUM8});
	map.insert({name_to_string("KP9\0"), win::button::NUM9});

	return map;
}

static const std::unordered_map<std::string, win::button> physical_keys = load_physical_keys();

static win::button name_to_button(const char *name)
{
	auto iterator = physical_keys.find(name_to_string(name));
	if(iterator == physical_keys.end())
	{
		std::cerr << "Unkown key \"" << name[0] << name[1] << name[2] << name[3] << "\"" << std::endl;
		return win::button::UNDEFINED;
	}

	return iterator->second;
}

win::display::display(const char *caption, int width, int height, int flags, window_handle parent)
{
	handler.key_button = handler_button;
	handler.character = handler_character;
	handler.mouse = handler_mouse;
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

	// fullscreen
	if(flags & FULLSCREEN)
		XChangeProperty(xdisplay, window_, XInternAtom(xdisplay, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (const unsigned char*)&atom_fullscreen, 1);

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
	XSetWMProtocols(xdisplay, window_, &atom_delete_window, 1);

	// vsync
	glXSwapIntervalEXT(xdisplay, window_, 1);

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
		if(xevent.xany.window != window_)
			return true;
		XNextEvent(xdisplay, &xevent);

		switch(xevent.type)
		{
			case ClientMessage: return false;

			case KeyPress:
			{
				handler.key_button(name_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), true);
				const KeySym sym = x_get_keysym(&xevent.xkey);
				if(sym)
					handler.character(sym, true);
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

				handler.key_button(name_to_button(xkb_desc->names->keys[xevent.xkey.keycode].name), false);
				const KeySym sym = x_get_keysym(&xevent.xkey);
				if(sym)
					handler.character(sym, false);
				break;
			}
			case MotionNotify:
				handler.mouse(xevent.xmotion.x, xevent.xmotion.y);
				break;
			case ButtonPress:
			case ButtonRelease:
				switch(xevent.xbutton.button)
				{
					case 1:
						handler.key_button(button::MOUSE_LEFT, xevent.type == ButtonPress);
						break;
					case 2:
						handler.key_button(button::MOUSE_MIDDLE, xevent.type == ButtonPress);
						break;
					case 3:
						handler.key_button(button::MOUSE_RIGHT, xevent.type == ButtonPress);
						break;
				}
				break;
		}
	}

	return true;
}

void win::display::swap() const
{
	glXSwapBuffers(xdisplay, window_);
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

	XGetGeometry(xdisplay, window_, &root, &xpos, &ypos, &width, &height, &border, &depth);

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

	XGetGeometry(xdisplay, window_, &root, &xpos, &ypos, &width, &height, &border, &depth);

	return height;
}

void win::display::cursor(bool show)
{
	if(show)
	{
		XUndefineCursor(xdisplay, window_);
	}
	else
	{
		Pixmap pm;
		XColor dummy;
		char data[2] = {0, 0};
		Cursor cursor;

		pm = XCreateBitmapFromData(xdisplay, window_, data, 1, 1);
		cursor = XCreatePixmapCursor(xdisplay, pm, pm, &dummy, &dummy, 0, 0);
		XFreePixmap(xdisplay, pm);

		XDefineCursor(xdisplay, window_, cursor);
	}
}

void win::display::event_button(fn_event_button f)
{
	handler.key_button = f;
	joystick_.event_button(f);
}

void win::display::event_joystick(fn_event_joystick f)
{
	joystick_.event_joystick(std::move(f));
}

void win::display::event_character(fn_event_character f)
{
	handler.character = std::move(f);
}

void win::display::event_mouse(fn_event_mouse f)
{
	handler.mouse = std::move(f);
}

int win::display::screen_width()
{
	return WidthOfScreen(ScreenOfDisplay(xdisplay, 0));
}

int win::display::screen_height()
{
	return HeightOfScreen(ScreenOfDisplay(xdisplay, 0));
}

void win::display::process_joystick()
{
	joystick_.process();
}

void win::display::move(display &rhs)
{
	handler.key_button = std::move(rhs.handler.key_button);
	handler.character = std::move(rhs.handler.character);
	handler.mouse = std::move(rhs.handler.mouse);
	joystick_ = std::move(rhs.joystick_);

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

// this function is taller than i am
void win::display::win_init_gl(HWND hwnd)
{
	hdc_ = GetDC(hwnd);

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

	SetPixelFormat(hdc_, ChoosePixelFormat(hdc_, &pfd), &pfd);
	HGLRC tmp = wglCreateContext(hdc_);
	wglMakeCurrent(hdc_, tmp);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (decltype(wglCreateContextAttribsARB))wglGetProcAddress("wglCreateContextAttribsARB");
	context_ = wglCreateContextAttribsARB(hdc_, NULL, attribs);
	wglMakeCurrent(hdc_, context_);
	wglDeleteContext(tmp);
	if(context_ == NULL)
	{
		ReleaseDC(hwnd, hdc_);
		MessageBox(NULL, "This software requires support for at least Opengl 3.3", "Fatal Error", MB_ICONEXCLAMATION);
		std::abort();
	}
	load_extensions();
}

void win::display::win_term_gl()
{
	wglMakeCurrent(hdc_, NULL);
	wglDeleteContext(context_);
	ReleaseDC(window_, hdc_);
}

LRESULT CALLBACK win::display::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT*)lp;
		win::indirect *d = (win::indirect*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)d);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		return TRUE;
	}

	win::indirect *const ind = (win::indirect*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(ind == NULL)
		return DefWindowProc(hwnd, msg, wp, lp);
	win::display *const dsp = (win::display*)ind->dsp;


	switch(msg)
	{
		case WM_CREATE:
			dsp->win_init_gl(hwnd);
			return 0;
		case WM_CHAR:
			if(wp >= ' ' && wp <= '~')
				dsp->handler.character(wp);
			return 0;
		case WM_CLOSE:
			dsp->winquit_ = true;
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

	indirect_.reset(new indirect(this));
	handler.key_button = handler_button;
	handler.character = handler_character;
	handler.mouse = handler_mouse;

	winquit_ = false;

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
		window_ = CreateWindowEx(0, window_class, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN), NULL, NULL, GetModuleHandle(NULL), indirect_.get());
	else
		window_ = CreateWindowEx(0, window_class, caption, WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, w, h, NULL, NULL, GetModuleHandle(NULL), indirect_.get());
	if(window_ == NULL)
		throw exception("Could not create window");

	SetWindowText(window_, caption);

	ShowWindow(window_, SW_SHOWDEFAULT);
	UpdateWindow(window_);
}

// return false if application is to exit
bool win::display::process()
{
	MSG msg;

	while(PeekMessage(&msg, window_, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return !winquit_;
}

void win::display::swap() const
{
	SwapBuffers(hdc_);
}

int win::display::width() const
{
	RECT rect;
	GetClientRect(window_, &rect);

	return rect.right;
}

int win::display::height() const
{
	RECT rect;
	GetClientRect(window_, &rect);

	return rect.bottom;
}

void win::display::cursor(bool)
{
}

void win::display::event_button(fn_event_button fn)
{
	handler.key_button = std::move(fn);
}

void win::display::event_joystick(fn_event_joystick)
{
}

void win::display::event_character(fn_event_character fn)
{
	handler.character = std::move(fn);
}

void win::display::event_mouse(fn_event_mouse fn)
{
	handler.mouse = std::move(fn);
}

int win::display::screen_width()
{
	return 0;
}

int win::display::screen_height()
{
	return 0;
}

void win::display::process_joystick()
{
}

void win::display::move(display &rhs)
{
	handler.key_button = std::move(rhs.handler.key_button);
	handler.character = std::move(rhs.handler.character);
	handler.mouse = std::move(rhs.handler.mouse);


	window_ = rhs.window_;
	rhs.window_ = NULL;

	indirect_ = std::move(rhs.indirect_);
	indirect_->dsp = this;

	hdc_ = rhs.hdc_;
	context_ = rhs.context_;
	winquit_ = rhs.winquit_;
}

void win::display::finalize()
{
	if(window_ == NULL)
		return;

	// close the window
	DestroyWindow(window_);
	window_ = NULL;
}

#else
#error "unsupported platform"
#endif
