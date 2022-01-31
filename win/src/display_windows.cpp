#include <win.h>

#ifdef WINPLAT_WINDOWS

#include <windowsx.h>

namespace win
{

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
		WGL_CONTEXT_MAJOR_VERSION_ARB, display.gl_major, WGL_CONTEXT_MINOR_VERSION_ARB, display.gl_minor, 0
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
		MessageBox(NULL, ("This software requires support for at least Opengl " + std::to_string(display.gl_major) + "." + std::to_string(display.gl_minor)).c_str(), "Fatal Error", MB_ICONEXCLAMATION);
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
		display.window_handler(WindowEvent::CLOSE);
		return 0;
	case WM_ERASEBKGND:
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	win::bug("late return from wndproc");
}

Display::Display(const DisplayOptions &options)
{
	const char *const window_class = "win_window_class";

	gl_major = options.gl_major;
	gl_minor = options.gl_minor;

	window_handler = default_window_handler;
	button_handler = default_button_handler;
	character_handler = default_character_handler;
	mouse_handler = default_mouse_handler;

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

	if(options.fullscreen)
		window = CreateWindowEx(0, window_class, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN), NULL, NULL, GetModuleHandle(NULL), this);
	else
		window = CreateWindowEx(0, window_class, options.caption.c_str(), WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, options.width, options.height, NULL, NULL, GetModuleHandle(NULL), this);
	if(window == NULL)
		win::bug("Could not create window");

	SetWindowText(window, options.caption.c_str());

	ShowWindow(window, SW_SHOWDEFAULT);

	if(!options.fullscreen)
	{
		RECT rect;
		GetClientRect(window, &rect);
		SetWindowPos(window, NULL, 0, 0, options.width + (options.width - rect.right), options.height + (options.height - rect.bottom), SWP_SHOWWINDOW);
	}

	glViewport(0, 0, options.width, options.height);

	UpdateWindow(window);
}

Display::~Display()
{
	win_term_gl();
	// close the window
	DestroyWindow(window);
}

void Display::process()
{
	MSG msg;

	while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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

void Display::register_window_handler(WindowHandler fn)
{
	window_handler = std::move(fn);
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

}

#endif
