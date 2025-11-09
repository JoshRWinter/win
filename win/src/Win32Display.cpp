#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <unordered_map>
#include <iostream>

#include <dxgi.h>

#include <win/Win32Display.hpp>

#ifdef WIN_USE_OPENGL
#include <win/GL/GL.hpp>
#endif

namespace win
{

static std::unordered_map<unsigned, win::Button> get_physical_keys()
{
	HKL current = GetKeyboardLayout(0);
	HKL qwerty = LoadKeyboardLayout("00000409", KLF_ACTIVATE);

	std::unordered_map<unsigned, win::Button> map;

	map.insert({MapVirtualKeyEx('A', MAPVK_VK_TO_VSC, qwerty), win::Button::a});
	map.insert({MapVirtualKeyEx('B', MAPVK_VK_TO_VSC, qwerty), win::Button::b});
	map.insert({MapVirtualKeyEx('C', MAPVK_VK_TO_VSC, qwerty), win::Button::c});
	map.insert({MapVirtualKeyEx('D', MAPVK_VK_TO_VSC, qwerty), win::Button::d});
	map.insert({MapVirtualKeyEx('E', MAPVK_VK_TO_VSC, qwerty), win::Button::e});
	map.insert({MapVirtualKeyEx('F', MAPVK_VK_TO_VSC, qwerty), win::Button::f});
	map.insert({MapVirtualKeyEx('G', MAPVK_VK_TO_VSC, qwerty), win::Button::g});
	map.insert({MapVirtualKeyEx('H', MAPVK_VK_TO_VSC, qwerty), win::Button::h});
	map.insert({MapVirtualKeyEx('I', MAPVK_VK_TO_VSC, qwerty), win::Button::i});
	map.insert({MapVirtualKeyEx('J', MAPVK_VK_TO_VSC, qwerty), win::Button::j});
	map.insert({MapVirtualKeyEx('K', MAPVK_VK_TO_VSC, qwerty), win::Button::k});
	map.insert({MapVirtualKeyEx('L', MAPVK_VK_TO_VSC, qwerty), win::Button::l});
	map.insert({MapVirtualKeyEx('M', MAPVK_VK_TO_VSC, qwerty), win::Button::m});
	map.insert({MapVirtualKeyEx('N', MAPVK_VK_TO_VSC, qwerty), win::Button::n});
	map.insert({MapVirtualKeyEx('O', MAPVK_VK_TO_VSC, qwerty), win::Button::o});
	map.insert({MapVirtualKeyEx('P', MAPVK_VK_TO_VSC, qwerty), win::Button::p});
	map.insert({MapVirtualKeyEx('Q', MAPVK_VK_TO_VSC, qwerty), win::Button::q});
	map.insert({MapVirtualKeyEx('R', MAPVK_VK_TO_VSC, qwerty), win::Button::r});
	map.insert({MapVirtualKeyEx('S', MAPVK_VK_TO_VSC, qwerty), win::Button::s});
	map.insert({MapVirtualKeyEx('T', MAPVK_VK_TO_VSC, qwerty), win::Button::t});
	map.insert({MapVirtualKeyEx('U', MAPVK_VK_TO_VSC, qwerty), win::Button::u});
	map.insert({MapVirtualKeyEx('V', MAPVK_VK_TO_VSC, qwerty), win::Button::v});
	map.insert({MapVirtualKeyEx('W', MAPVK_VK_TO_VSC, qwerty), win::Button::w});
	map.insert({MapVirtualKeyEx('X', MAPVK_VK_TO_VSC, qwerty), win::Button::x});
	map.insert({MapVirtualKeyEx('Y', MAPVK_VK_TO_VSC, qwerty), win::Button::y});
	map.insert({MapVirtualKeyEx('Z', MAPVK_VK_TO_VSC, qwerty), win::Button::z});

	map.insert({MapVirtualKeyEx(0x30, MAPVK_VK_TO_VSC, qwerty), win::Button::d0});
	map.insert({MapVirtualKeyEx(0x31, MAPVK_VK_TO_VSC, qwerty), win::Button::d1});
	map.insert({MapVirtualKeyEx(0x32, MAPVK_VK_TO_VSC, qwerty), win::Button::d2});
	map.insert({MapVirtualKeyEx(0x33, MAPVK_VK_TO_VSC, qwerty), win::Button::d3});
	map.insert({MapVirtualKeyEx(0x34, MAPVK_VK_TO_VSC, qwerty), win::Button::d4});
	map.insert({MapVirtualKeyEx(0x35, MAPVK_VK_TO_VSC, qwerty), win::Button::d5});
	map.insert({MapVirtualKeyEx(0x36, MAPVK_VK_TO_VSC, qwerty), win::Button::d6});
	map.insert({MapVirtualKeyEx(0x37, MAPVK_VK_TO_VSC, qwerty), win::Button::d7});
	map.insert({MapVirtualKeyEx(0x38, MAPVK_VK_TO_VSC, qwerty), win::Button::d8});
	map.insert({MapVirtualKeyEx(0x39, MAPVK_VK_TO_VSC, qwerty), win::Button::d9});

	map.insert({MapVirtualKeyEx(VK_OEM_3, MAPVK_VK_TO_VSC, qwerty), win::Button::backtick});
	map.insert({MapVirtualKeyEx(VK_OEM_MINUS, MAPVK_VK_TO_VSC, qwerty), win::Button::dash});
	map.insert({MapVirtualKeyEx(VK_OEM_PLUS, MAPVK_VK_TO_VSC, qwerty), win::Button::equals});
	map.insert({MapVirtualKeyEx(VK_OEM_4, MAPVK_VK_TO_VSC, qwerty), win::Button::lbracket});
	map.insert({MapVirtualKeyEx(VK_OEM_6, MAPVK_VK_TO_VSC, qwerty), win::Button::rbracket});
	map.insert({MapVirtualKeyEx(VK_OEM_1, MAPVK_VK_TO_VSC, qwerty), win::Button::semicolon});
	map.insert({MapVirtualKeyEx(VK_OEM_7, MAPVK_VK_TO_VSC, qwerty), win::Button::apostrophe});
	map.insert({MapVirtualKeyEx(VK_OEM_COMMA, MAPVK_VK_TO_VSC, qwerty), win::Button::comma});
	map.insert({MapVirtualKeyEx(VK_OEM_PERIOD, MAPVK_VK_TO_VSC, qwerty), win::Button::period});
	map.insert({MapVirtualKeyEx(VK_OEM_2, MAPVK_VK_TO_VSC, qwerty), win::Button::slash});
	map.insert({MapVirtualKeyEx(VK_OEM_102, MAPVK_VK_TO_VSC, qwerty), win::Button::backslash});

	map.insert({MapVirtualKeyEx(VK_F1, MAPVK_VK_TO_VSC, qwerty), win::Button::f1});
	map.insert({MapVirtualKeyEx(VK_F2, MAPVK_VK_TO_VSC, qwerty), win::Button::f2});
	map.insert({MapVirtualKeyEx(VK_F3, MAPVK_VK_TO_VSC, qwerty), win::Button::f3});
	map.insert({MapVirtualKeyEx(VK_F4, MAPVK_VK_TO_VSC, qwerty), win::Button::f4});
	map.insert({MapVirtualKeyEx(VK_F5, MAPVK_VK_TO_VSC, qwerty), win::Button::f5});
	map.insert({MapVirtualKeyEx(VK_F6, MAPVK_VK_TO_VSC, qwerty), win::Button::f6});
	map.insert({MapVirtualKeyEx(VK_F7, MAPVK_VK_TO_VSC, qwerty), win::Button::f7});
	map.insert({MapVirtualKeyEx(VK_F8, MAPVK_VK_TO_VSC, qwerty), win::Button::f8});
	map.insert({MapVirtualKeyEx(VK_F9, MAPVK_VK_TO_VSC, qwerty), win::Button::f9});
	map.insert({MapVirtualKeyEx(VK_F10, MAPVK_VK_TO_VSC, qwerty), win::Button::f10});
	map.insert({MapVirtualKeyEx(VK_F11, MAPVK_VK_TO_VSC, qwerty), win::Button::f11});
	map.insert({MapVirtualKeyEx(VK_F12, MAPVK_VK_TO_VSC, qwerty), win::Button::f12});

	map.insert({MapVirtualKeyEx(VK_ESCAPE, MAPVK_VK_TO_VSC, qwerty), win::Button::esc});
	// map.insert({MapVirtualKeyEx(VK_SNAPSHOT, MAPVK_VK_TO_VSC, qwerty), win::Button::print_sCR});
	// map.insert({MapVirtualKeyEx(VK_CANCEL, MAPVK_VK_TO_VSC, qwerty), win::Button::pause_brEAK});
	map.insert({MapVirtualKeyEx(VK_INSERT, MAPVK_VK_TO_VSC, qwerty), win::Button::insert});
	map.insert({MapVirtualKeyEx(VK_DELETE, MAPVK_VK_TO_VSC, qwerty), win::Button::del});
	map.insert({MapVirtualKeyEx(VK_HOME, MAPVK_VK_TO_VSC, qwerty), win::Button::home});
	map.insert({MapVirtualKeyEx(VK_PRIOR, MAPVK_VK_TO_VSC, qwerty), win::Button::page_up});
	map.insert({MapVirtualKeyEx(VK_NEXT, MAPVK_VK_TO_VSC, qwerty), win::Button::page_down});
	map.insert({MapVirtualKeyEx(VK_END, MAPVK_VK_TO_VSC, qwerty), win::Button::end});
	map.insert({MapVirtualKeyEx(VK_BACK, MAPVK_VK_TO_VSC, qwerty), win::Button::backspace});
	map.insert({MapVirtualKeyEx(VK_RETURN, MAPVK_VK_TO_VSC, qwerty), win::Button::ret});
	// map.insert({MapVirtualKeyEx(VK_EXECUTE, MAPVK_VK_TO_VSC, qwerty), win::Button::enter});
	map.insert({MapVirtualKeyEx(VK_LSHIFT, MAPVK_VK_TO_VSC, qwerty), win::Button::lshift});
	map.insert({MapVirtualKeyEx(VK_RSHIFT, MAPVK_VK_TO_VSC, qwerty), win::Button::rshift});
	map.insert({MapVirtualKeyEx(VK_LCONTROL, MAPVK_VK_TO_VSC, qwerty), win::Button::lctrl});
	map.insert({MapVirtualKeyEx(VK_RCONTROL, MAPVK_VK_TO_VSC, qwerty), win::Button::rctrl});
	map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::Button::lalt});
	// map.insert({MapVirtualKeyEx(VK_MENU, MAPVK_VK_TO_VSC, qwerty), win::Button::lalt});
	map.insert({MapVirtualKeyEx(VK_SPACE, MAPVK_VK_TO_VSC, qwerty), win::Button::space});
	// map.insert({MapVirtualKeyEx(VK_RMENU, MAPVK_VK_TO_VSC, qwerty), win::Button::lmeta});
	map.insert({MapVirtualKeyEx(VK_LWIN, MAPVK_VK_TO_VSC, qwerty), win::Button::lmeta});
	map.insert({MapVirtualKeyEx(VK_RWIN, MAPVK_VK_TO_VSC, qwerty), win::Button::rmeta});
	map.insert({MapVirtualKeyEx(VK_UP, MAPVK_VK_TO_VSC, qwerty), win::Button::up});
	map.insert({MapVirtualKeyEx(VK_DOWN, MAPVK_VK_TO_VSC, qwerty), win::Button::down});
	map.insert({MapVirtualKeyEx(VK_LEFT, MAPVK_VK_TO_VSC, qwerty), win::Button::left});
	map.insert({MapVirtualKeyEx(VK_RIGHT, MAPVK_VK_TO_VSC, qwerty), win::Button::right});
	map.insert({MapVirtualKeyEx(VK_CAPITAL, MAPVK_VK_TO_VSC, qwerty), win::Button::capslock});
	map.insert({MapVirtualKeyEx(VK_TAB, MAPVK_VK_TO_VSC, qwerty), win::Button::tab});

	map.insert({MapVirtualKeyEx(VK_NUMLOCK, MAPVK_VK_TO_VSC, qwerty), win::Button::num_lock});
	map.insert({MapVirtualKeyEx(VK_DIVIDE, MAPVK_VK_TO_VSC, qwerty), win::Button::num_slash});
	map.insert({MapVirtualKeyEx(VK_MULTIPLY, MAPVK_VK_TO_VSC, qwerty), win::Button::num_multiply});
	map.insert({MapVirtualKeyEx(VK_SUBTRACT, MAPVK_VK_TO_VSC, qwerty), win::Button::num_minus});
	map.insert({MapVirtualKeyEx(VK_ADD, MAPVK_VK_TO_VSC, qwerty), win::Button::num_plus});
	map.insert({MapVirtualKeyEx(VK_NUMPAD0, MAPVK_VK_TO_VSC, qwerty), win::Button::num0});
	map.insert({MapVirtualKeyEx(VK_NUMPAD1, MAPVK_VK_TO_VSC, qwerty), win::Button::num1});
	map.insert({MapVirtualKeyEx(VK_NUMPAD2, MAPVK_VK_TO_VSC, qwerty), win::Button::num2});
	map.insert({MapVirtualKeyEx(VK_NUMPAD3, MAPVK_VK_TO_VSC, qwerty), win::Button::num3});
	map.insert({MapVirtualKeyEx(VK_NUMPAD4, MAPVK_VK_TO_VSC, qwerty), win::Button::num4});
	map.insert({MapVirtualKeyEx(VK_NUMPAD5, MAPVK_VK_TO_VSC, qwerty), win::Button::num5});
	map.insert({MapVirtualKeyEx(VK_NUMPAD6, MAPVK_VK_TO_VSC, qwerty), win::Button::num6});
	map.insert({MapVirtualKeyEx(VK_NUMPAD7, MAPVK_VK_TO_VSC, qwerty), win::Button::num7});
	map.insert({MapVirtualKeyEx(VK_NUMPAD8, MAPVK_VK_TO_VSC, qwerty), win::Button::num8});
	map.insert({MapVirtualKeyEx(VK_NUMPAD9, MAPVK_VK_TO_VSC, qwerty), win::Button::num9});

	ActivateKeyboardLayout(current, KLF_RESET);
	return map;
}

static std::unordered_map<unsigned, win::Button> physical_keys = get_physical_keys();

static win::Button get_physical_key(unsigned scan)
{
	const auto it = physical_keys.find(scan);
	if(it == physical_keys.end())
		return win::Button::undefined;

	return it->second;
}

void Win32Display::win_init_gl(HWND hwnd)
{
	hdc = GetDC(hwnd);

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
		WGL_CONTEXT_MAJOR_VERSION_ARB, options.gl_major, WGL_CONTEXT_MINOR_VERSION_ARB, options.gl_minor, 0
	};

	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
	HGLRC tmp = wglCreateContext(hdc);
	wglMakeCurrent(hdc, tmp);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (decltype(wglCreateContextAttribsARB))wglGetProcAddress("wglCreateContextAttribsARB");
	context = wglCreateContextAttribsARB(hdc, NULL, attribs);
	wglMakeCurrent(hdc, context);
	wglDeleteContext(tmp);
	if(context == NULL)
	{
		ReleaseDC(hwnd, hdc);
		MessageBox(NULL, ("This software requires support for at least Opengl " + std::to_string(options.gl_major) + "." + std::to_string(options.gl_minor)).c_str(), "Fatal Error", MB_ICONEXCLAMATION);
		std::abort();
	}

	wglSwapIntervalEXT = (decltype(wglSwapIntervalEXT)) wglGetProcAddress("wglSwapIntervalEXT");
}

void Win32Display::win_term_gl()
{
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(context);
	ReleaseDC(window, hdc);
}

LRESULT CALLBACK Win32Display::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if(msg == WM_NCCREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT*)lp;
		Win32Display *d = (win::Win32Display*)cs->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)d);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		return TRUE;
	}

	Win32Display *const display_ptr = (win::Win32Display*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if(display_ptr == NULL)
		return DefWindowProc(hwnd, msg, wp, lp);

	Win32Display &display = *display_ptr;

	switch(msg)
	{
		case WM_CREATE:
			display.win_init_gl(hwnd);
			return 0;
		case WM_CHAR:
			if(wp >= ' ' && wp <= '~')
				display.character_handler(wp);
			return 0;
		case WM_KEYDOWN:
		{
			const unsigned scancode = (lp >> 16) & 0xff;
			const Button key = get_physical_key(scancode);
			if(key == Button::undefined)
			{
				std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
				return 0;
			}

			display.button_handler(key, true);
			return 0;
		}
		case WM_KEYUP:
		{
			const unsigned scancode = (lp >> 16) & 0xff;
			const Button key = get_physical_key(scancode);
			if(key == Button::undefined)
			{
				std::cerr << "Unrecognized virtual key " << wp << " scancode " << scancode << std::endl;
				return 0;
			}

			display.button_handler(key, false);
			return 0;
		}
		case WM_SYSCOMMAND:
			if(wp != SC_KEYMENU)
				return DefWindowProc(hwnd, msg, wp, lp);
		case WM_MOUSEMOVE:
			display.mouse_handler(LOWORD(lp), HIWORD(lp));
			return 0;
		case WM_LBUTTONDOWN:
			display.button_handler(Button::mouse_left, true);
			return 0;
		case WM_LBUTTONUP:
			display.button_handler(Button::mouse_left, false);
			return 0;
		case WM_RBUTTONDOWN:
			display.button_handler(Button::mouse_right, true);
			return 0;
		case WM_RBUTTONUP:
			display.button_handler(Button::mouse_right, false);
			return 0;
		case WM_MBUTTONDOWN:
			display.button_handler(Button::mouse_middle, true);
			return 0;
		case WM_MBUTTONUP:
			display.button_handler(Button::mouse_middle, false);
			return 0;
		case WM_CLOSE:
			display.window_handler(WindowEvent::close);
			return 0;
		case WM_ERASEBKGND:
			return 0;
		case WM_WINDOWPOSCHANGED:
			return DefWindowProc(hwnd, msg, wp, lp);
		case WM_SIZE:
		{
			const auto w = LOWORD(lp);
			const auto h = HIWORD(lp);

			display.update_refresh_rate();

			if (display.window_prop_cache.w != w || display.window_prop_cache.h != h)
			{
				display.window_prop_cache.w = w;
				display.window_prop_cache.h = h;

				display.resize_handler(w, h);
			}

			return 0;
		}
		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}

	win::bug("late return from wndproc");
}

Win32Display::Win32Display(const DisplayOptions &options)
	: options(options)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
	init_monitor_properties();

	const char *const window_class = "win_window_class";

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

	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	if (monitor == NULL)
		win::bug("MonitorFromWindow failure");

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(monitor, &mi))
		win::bug("GetMonitorInfo failure");

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	if (!EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm))
		win::bug("EnumDisplaySettings failure");

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = options.fullscreen ? dm.dmPelsWidth : options.width;
	rect.bottom = options.fullscreen ? dm.dmPelsHeight : options.height;

	if (!AdjustWindowRectEx(&rect, options.fullscreen ? fullscreen_style : windowed_style, FALSE, 0))
		win::bug("AdjustWindowRectExFailure");

	window = CreateWindowEx(
		0,
		window_class,
		options.caption.c_str(),
		options.fullscreen ? fullscreen_style : windowed_style,
		options.fullscreen ? mi.rcMonitor.left : CW_USEDEFAULT,
		options.fullscreen ? mi.rcMonitor.top : CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		options.parent,
		NULL,
		GetModuleHandle(NULL),
		this);

	if(window == NULL)
		win::bug("Could not create window");

	SetWindowText(window, options.caption.c_str());

	ShowWindow(window, SW_SHOWDEFAULT);

	if (!GetClientRect(window, &rect))
		win::bug("GetClientRect failure");

	glViewport(0, 0, rect.right, rect.bottom);

	window_prop_cache.w = rect.right - rect.left;
	window_prop_cache.h = rect.bottom - rect.top;

	update_refresh_rate();
}

Win32Display::~Win32Display()
{
	win_term_gl();
	// close the window
	DestroyWindow(window);
}

void Win32Display::process()
{
	MSG msg;

	while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Win32Display::swap()
{
	SwapBuffers(hdc);
}

int Win32Display::width()
{
	RECT rect;
	GetClientRect(window, &rect);

	return rect.right;
}

int Win32Display::height()
{
	RECT rect;
	GetClientRect(window, &rect);

	return rect.bottom;
}

void Win32Display::resize(int w, int h)
{
	const auto style = GetWindowLongA(window, GWL_STYLE);

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = w;
	rect.bottom = h;

	const auto s = style & WS_MINIMIZEBOX ? windowed_style : fullscreen_style;

	if (!AdjustWindowRectEx(&rect, s, FALSE, 0))
		win::bug("AdjustWindowRectEx failure");

	if (!SetWindowPos(window, HWND_TOP, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_FRAMECHANGED))
		win::bug("SetWindowPos failure");

	PostMessage(window, WM_EXITSIZEMOVE, 0, 0);
}

float Win32Display::refresh_rate()
{
	return rrate;
}

void Win32Display::cursor(bool show)
{
	ShowCursor(show);
}

void Win32Display::set_fullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (monitor == NULL)
			win::bug("MonitorFromWindow failure");

		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		if (!GetMonitorInfo(monitor, &mi))
			win::bug("GetMonitorInfo failure");

		DEVMODE dm;
		dm.dmSize = sizeof(dm);
		if (!EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm))
			win::bug("EnumDisplaySettings failure");

		SetWindowLongPtrA(window, GWL_STYLE, fullscreen_style);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = dm.dmPelsWidth;
		rect.bottom = dm.dmPelsHeight;

		if (!AdjustWindowRectEx(&rect, fullscreen_style, FALSE, 0))
			win::bug("AdjustWindowRectEx failure");

		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLongPtrA(window, GWL_STYLE, windowed_style);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = options.width;
		rect.bottom = options.height;

		if (!AdjustWindowRectEx(&rect, windowed_style, FALSE, 0))
			win::bug("AdjustWindowRectEx failure");

		SetWindowPos(window, HWND_TOP, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_FRAMECHANGED);
	}

	ShowWindow(window, SW_SHOWDEFAULT);

	PostMessage(window, WM_EXITSIZEMOVE, 0, 0);
}

void Win32Display::vsync(bool on)
{
	wglSwapIntervalEXT(on);
}

NativeWindowHandle Win32Display::native_handle()
{
	return window;
}

void Win32Display::init_monitor_properties()
{
	monprops.clear();

	IDXGIFactory1 *factory;
	HRESULT result;

	std::vector<DXGI_MODE_DESC> modes;

	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);

	if (result != S_OK)
	{
		fprintf(stderr, "CreateDXGIFactory1 failed: %ld\n", result);
		return;
	}

	UINT i = 0;
	IDXGIAdapter1 *adapter;
	while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		UINT i2 = 0;
		IDXGIOutput *output;
		while (adapter->EnumOutputs(i2, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			result = output->GetDesc(&desc);

			if (result != S_OK)
			{
				fprintf(stderr, "IDXGIOutput::GetDesc failed: %ld\n", result);
				continue;
			}

			UINT num = 0;
			result = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, NULL);

			if (result != S_OK)
			{
				fprintf(stderr, "IDXGIOutput::GetDisplayModeList failed: %ld\n", result);
				continue;
			}

			modes.resize(num);
			result = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, modes.data());

			if (result != S_OK)
			{
				fprintf(stderr, "IDXGIOutput::GetDisplayModeList failed: %ld\n", result);
				continue;
			}

			for (const auto &mode : modes)
			{
				auto &props = monprops.emplace_back();

				static_assert(sizeof(desc.DeviceName) == sizeof(WCHAR) * 32, "DXGI_OUTPUT_DESC::DeviceName must be 32 WCHAR array");

				// Turrible.
				char name[33];
				for (int j = 0; j < 32; ++j)
					name[j] = desc.DeviceName[j];

				name[32] = 0;

				props.name = name;
				props.width = mode.Width;
				props.height = mode.Height;
				props.rate = mode.RefreshRate.Numerator / (float)mode.RefreshRate.Denominator;
			}

			output->Release();
			++i2;
		}

		adapter->Release();
		++i;
	}

	factory->Release();
}

void Win32Display::update_refresh_rate()
{
	HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);

	if (monitor == NULL)
	{
		fprintf(stderr, "MonitorFromWindow() returned NULL\n");
		rrate = 60.0f;
		return;
	}

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);

	if (!GetMonitorInfo(monitor, &mi))
	{
		fprintf(stderr, "GetMonitorInfo() returned NULL\n");
		rrate = 60.0f;
		return;
	}

	DEVMODE dm;
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm))
	{
		fprintf(stderr, "GetMonitorInfo() returned NULL\n");
		rrate = 60.0f;
		return;
	}

	const MonitorProperties* closest = NULL;

	// Iterate over monprops to find the closest one.
	// The alleged refreshrate given in dm.dmDisplayFrequency is often rounded, we are trying to suss out which one it actually is.
	for (const auto& mode : monprops)
	{
		if (mode.name == mi.szDevice && mode.width == dm.dmPelsWidth && mode.height == dm.dmPelsHeight)
		{
			if (closest == NULL)
				closest = &mode;
			else if (std::fabsf(mode.rate - dm.dmDisplayFrequency) < std::fabs(closest->rate - dm.dmDisplayFrequency))
				closest = &mode;
		}
	}

	if (closest == NULL)
	{
		fprintf(stderr, "Couldn't match the display mode.\n");
		rrate = 60.0f;
	}
	else
	{
		rrate = closest->rate;
	}
}

}

#endif
