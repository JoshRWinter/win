#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <dxgi.h>
#include <windowsx.h>
//#include <dwmapi.h>

#include <win/Win32Display.hpp>

#ifdef WIN_USE_OPENGL
#include <win/GL/GL.hpp>
#endif

namespace win
{

void Win32Display::win_init_gl(HWND hwnd)
{
    hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    //pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, options.gl_major, WGL_CONTEXT_MINOR_VERSION_ARB, options.gl_minor, 0 };

    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
    HGLRC tmp = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tmp);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (decltype(wglCreateContextAttribsARB))wglGetProcAddress("wglCreateContextAttribsARB");
    context = wglCreateContextAttribsARB(hdc, NULL, attribs);
    wglMakeCurrent(hdc, context);
    wglDeleteContext(tmp);
    if (context == NULL)
    {
        ReleaseDC(hwnd, hdc);
        MessageBox(NULL,
                   ("This software requires support for at least Opengl " + std::to_string(options.gl_major) + "." + std::to_string(options.gl_minor)).c_str(),
                   "Fatal Error",
                   MB_ICONEXCLAMATION);
        std::abort();
    }

    wglSwapIntervalEXT = (decltype(wglSwapIntervalEXT))wglGetProcAddress("wglSwapIntervalEXT");
}

void Win32Display::win_term_gl()
{
    wglMakeCurrent(hdc, NULL);
    wglDeleteContext(context);
    ReleaseDC(window, hdc);
}

LRESULT CALLBACK Win32Display::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT *cs = (CREATESTRUCT *)lp;
        Win32Display *d = (win::Win32Display *)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)d);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

        return TRUE;
    }

    Win32Display *const display_ptr = (win::Win32Display *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (display_ptr == NULL)
        return DefWindowProc(hwnd, msg, wp, lp);

    Win32Display &display = *display_ptr;

    switch (msg)
    {
        case WM_CREATE:
            display.win_init_gl(hwnd);
            return 0;
        case WM_CHAR:
            if (wp >= ' ' && wp <= '~')
                display.character_handler(wp);
            return 0;
        case WM_KEYDOWN:
        {
            const unsigned scancode = (lp >> 16) & 0xff;
            if (scancode < display.keys.size())
            {
                const Button key = display.keys[scancode];
                if (key != Button::undefined)
                {
                    if (!display.keystates[scancode])
                    {
                        display.keystates[scancode] = true;
                        display.button_handler(key, true);
                    }

                    return 0;
                }
            }

            return DefWindowProc(hwnd, msg, wp, lp);
        }
        case WM_KEYUP:
        {
            const unsigned scancode = (lp >> 16) & 0xff;
            if (scancode < display.keys.size())
            {
                const Button key = display.keys[scancode];
                if (key != Button::undefined)
                {
                    display.keystates[scancode] = false;
                    display.button_handler(key, false);

                    return 0;
                }
            }

            return DefWindowProc(hwnd, msg, wp, lp);
        }
        // case WM_SYSCOMMAND:
        //	if(wp != SC_KEYMENU)
        //		return DefWindowProc(hwnd, msg, wp, lp);
        //	break;
        case WM_MOUSEMOVE:
            display.mouse_handler(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
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
        case WM_MOVE:
            if (display.pointer_locked)
                display.lock_pointer();
            display.update_refresh_rate();
            return 0;
        case WM_SIZE:
        {
            const auto w = LOWORD(lp);
            const auto h = HIWORD(lp);

            display.update_refresh_rate();

            if (w != 0 || h != 0)
            {
                if (display.window_prop_cache.w != w || display.window_prop_cache.h != h)
                {
                    display.window_prop_cache.w = w;
                    display.window_prop_cache.h = h;

                    display.resize_state.resize = true;
                    display.resize_state.time = std::chrono::steady_clock::now();
                }
            }

            if (display.pointer_locked)
                display.lock_pointer();

            return 0;
        }
        case WM_ACTIVATE:
            if (display.pointer_locked)
            {
                if (LOWORD(wp) == WA_INACTIVE)
                    ClipCursor(NULL);
                else
                    display.lock_pointer();
            }
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }

    win::bug("late return from wndproc");
}

Win32Display::Win32Display(const DisplayOptions &options)
    : options(options)
{
    get_keys();

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    const char *const window_class = "win_window_class";

    HICON icon = LoadIcon(GetModuleHandle(NULL), "IDI_ICON1");
    icon = icon != NULL ? icon : LoadIcon(NULL, IDI_APPLICATION);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = window_class;
    wc.hIcon = icon;
    wc.hIconSm = icon;

    if (!RegisterClassEx(&wc))
    {
        if (GetLastError() == 1410) // already registered
        {
            if (!GetClassInfoEx(GetModuleHandle(NULL), window_class, &wc))
            {
                win::bug("Couldn't load existing window class: " + std::to_string(GetLastError()));
            }
        }
    }

    if (monitors.count() == 0)
        win::bug("No suitable monitors found.");

    const win::Monitor *primary_monitor = NULL;
    const win::Monitor *desired_monitor = NULL;

    for (const auto &monitor : monitors)
    {
        if (monitor.primary)
            primary_monitor = &monitor;
        if (!options.monitor_name.empty() && monitor.id == options.monitor_name)
            desired_monitor = &monitor;
    }

    if (primary_monitor == NULL)
        primary_monitor = &(*monitors.begin());

    DWORD style;
    int x, y;
    int w, h;

    if (options.parent != NULL)
    {
        style = WS_CHILD;
        x = 0;
        y = 0;

        RECT r;
        if (!GetWindowRect(options.parent, &r))
            win::bug("Couldn't get dimensions of parent window.");

        w = r.right - r.left;
        h = r.bottom - r.top;
    }
    else if (!options.monitor_name.empty())
    {
        if (desired_monitor == NULL)
            win::bug("Can't open display on monitor " + options.monitor_name + ": not found");

        style = options.fullscreen ? fullscreen_style : windowed_style;

        if (options.fullscreen)
        {
            style = fullscreen_style;

            x = desired_monitor->x;
            y = desired_monitor->y;
            w = desired_monitor->width;
            h = desired_monitor->height;
        }
        else
        {
            style = windowed_style;

            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = options.width;
            rect.bottom = options.height;

            if (!AdjustWindowRectEx(&rect, style, FALSE, 0))
                win::bug("AdjustWindowRectExFailure");

            w = rect.right - rect.left;
            h = rect.bottom - rect.top;
            x = (desired_monitor->x + (desired_monitor->width / 2)) - (w / 2);
            y = (desired_monitor->y + (desired_monitor->height / 2)) - (h / 2);
        }
    }
    else if (options.fullscreen)
    {
        style = fullscreen_style;

        x = primary_monitor->x;
        y = primary_monitor->y;
        w = primary_monitor->width;
        h = primary_monitor->height;
    }
    else
    {
        style = windowed_style;

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = options.width;
        rect.bottom = options.height;

        if (!AdjustWindowRectEx(&rect, style, FALSE, 0))
            win::bug("AdjustWindowRectExFailure");

        x = CW_USEDEFAULT;
        y = CW_USEDEFAULT;
        w = rect.right - rect.left;
        h = rect.bottom - rect.top;
    }

    window = CreateWindowEx(0, window_class, options.caption.c_str(), style, x, y, w, h, options.parent, NULL, GetModuleHandle(NULL), this);

    /*
    DWM_BLURBEHIND bb = {0};
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
    bb.fEnable = TRUE;
    DwmEnableBlurBehindWindow(window, &bb);
    */

    if (window == NULL)
        win::bug("Could not create window");

    SetWindowText(window, options.caption.c_str());

    RECT rect;
    if (!GetClientRect(window, &rect))
        win::bug("GetClientRect failure");

    window_prop_cache.w = rect.right - rect.left;
    window_prop_cache.h = rect.bottom - rect.top;

    ShowWindow(window, SW_SHOWDEFAULT);

    glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);

    update_refresh_rate();

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 1;
    rid.usUsage = 2;
    rid.dwFlags = 0;
    rid.hwndTarget = window;
    if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
        win::bug("Win32Display: Couldn't register raw input devices");

    raw_input.resize(16);
}

Win32Display::~Win32Display()
{
    win_term_gl();
    // close the window
    DestroyWindow(window);
}

void Win32Display::process()
{
    if (resize_state.resize && std::chrono::duration<float>(std::chrono::steady_clock::now() - resize_state.time).count() > 0.25f)
    {
        resize_state.resize = false;
        resize_handler(window_prop_cache.w, window_prop_cache.h);
    }

    MSG msg;

    while (PeekMessage(&msg, window, 0, WM_INPUT - 1, PM_REMOVE) || PeekMessage(&msg, window, WM_INPUT + 1, 0xFFFF, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    process_raw_mouse();
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

void Win32Display::show_pointer(bool show)
{
    ShowCursor(show);
}

void Win32Display::lock_pointer(bool lock)
{
    pointer_locked = lock;
    if (lock)
        lock_pointer();
    else
        ClipCursor(NULL);
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

void Win32Display::update_refresh_rate()
{
    HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);

    if (monitor == NULL)
    {
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

    const Monitor *m = NULL;
    for (const auto &mon : monitors)
    {
        if (!strcmp(mon.id.c_str(), mi.szDevice))
        {
            m = &mon;
            break;
        }
    }

    if (m == NULL)
    {
        fprintf(stderr, "Couldn't find refresh rate of monitor %s\n", mi.szDevice);
        rrate = 60.0f;
        return;
    }

    rrate = m->rate;
}

void Win32Display::process_raw_mouse()
{
    int x = 0, y = 0;

    while (true)
    {
        UINT size = sizeof(RAWINPUT) * raw_input.size();
        const auto count = GetRawInputBuffer(raw_input.data(), &size, sizeof(RAWINPUT::header));

        if (count == 0)
            break;

        if (count == (UINT)-1)
        {
            const auto e = GetLastError();
            if (e == 122)
            {
                raw_input.resize(raw_input.size() * 2);
                continue;
            }

            return; // just bug out
        }

        for (int i = 0; i < count; ++i)
        {
            if (raw_input[i].header.dwType == RIM_TYPEMOUSE)
            {
                if ((raw_input[i].data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
                {
                    x += raw_input[i].data.mouse.lLastX;
                    y += raw_input[i].data.mouse.lLastY;
                }
            }
        }
    }

    if (x != 0 || y != 0)
    {
        relative_mouse_handler(x, y);
    }
}

void Win32Display::lock_pointer()
{
    const auto exstyle = GetWindowExStyle(window);
    const auto style = GetWindowStyle(window);

    RECT rect;
    GetWindowRect(window, &rect);

    RECT rect2 = rect;
    AdjustWindowRectEx(&rect2, style, FALSE, exstyle);

    rect.left -= rect2.left - rect.left;
    rect.right -= rect2.right - rect.right;
    rect.bottom -= rect2.bottom - rect.bottom;
    rect.top -= rect2.top - rect.top;

    ClipCursor(&rect);
}

void Win32Display::get_keys()
{
    for (auto &x : keys)
        x = win::Button::undefined;

    for (auto &x : keystates)
        x = false;

    keys.at(1) = win::Button::esc;
    keys.at(2) = win::Button::d1;
    keys.at(3) = win::Button::d2;
    keys.at(4) = win::Button::d3;
    keys.at(5) = win::Button::d4;
    keys.at(6) = win::Button::d5;
    keys.at(7) = win::Button::d6;
    keys.at(8) = win::Button::d7;
    keys.at(9) = win::Button::d8;
    keys.at(10) = win::Button::d9;
    keys.at(11) = win::Button::d0;
    keys.at(12) = win::Button::dash;
    keys.at(13) = win::Button::equal;
    keys.at(14) = win::Button::backspace;
    keys.at(15) = win::Button::tab;
    keys.at(16) = win::Button::q;
    keys.at(17) = win::Button::w;
    keys.at(18) = win::Button::e;
    keys.at(19) = win::Button::r;
    keys.at(20) = win::Button::t;
    keys.at(21) = win::Button::y;
    keys.at(22) = win::Button::u;
    keys.at(23) = win::Button::i;
    keys.at(24) = win::Button::o;
    keys.at(25) = win::Button::p;
    keys.at(26) = win::Button::lbracket;
    keys.at(27) = win::Button::rbracket;
    keys.at(28) = win::Button::enter;
    keys.at(29) = win::Button::lctrl;
    keys.at(30) = win::Button::a;
    keys.at(31) = win::Button::s;
    keys.at(32) = win::Button::d;
    keys.at(33) = win::Button::f;
    keys.at(34) = win::Button::g;
    keys.at(35) = win::Button::h;
    keys.at(36) = win::Button::j;
    keys.at(37) = win::Button::k;
    keys.at(38) = win::Button::l;
    keys.at(39) = win::Button::semicolon;
    keys.at(40) = win::Button::apostrophe;
    keys.at(41) = win::Button::backtick;
    keys.at(42) = win::Button::lshift;
    keys.at(43) = win::Button::backslash;
    keys.at(44) = win::Button::z;
    keys.at(45) = win::Button::x;
    keys.at(46) = win::Button::c;
    keys.at(47) = win::Button::v;
    keys.at(48) = win::Button::b;
    keys.at(49) = win::Button::n;
    keys.at(50) = win::Button::m;
    keys.at(51) = win::Button::comma;
    keys.at(52) = win::Button::period;
    keys.at(53) = win::Button::slash;
    keys.at(54) = win::Button::rshift;
    keys.at(55) = win::Button::num_star;
    keys.at(56) = win::Button::lalt;
    keys.at(57) = win::Button::space;
    keys.at(58) = win::Button::capslock;
    keys.at(59) = win::Button::f1;
    keys.at(60) = win::Button::f2;
    keys.at(61) = win::Button::f3;
    keys.at(62) = win::Button::f4;
    keys.at(63) = win::Button::f5;
    keys.at(64) = win::Button::f6;
    keys.at(65) = win::Button::f7;
    keys.at(66) = win::Button::f8;
    keys.at(67) = win::Button::f9;
    keys.at(68) = win::Button::f10;
    keys.at(69) = win::Button::numlock;
    keys.at(71) = win::Button::home;
    keys.at(72) = win::Button::up;
    keys.at(73) = win::Button::page_up;
    keys.at(74) = win::Button::num_minus;
    keys.at(75) = win::Button::left;
    keys.at(76) = win::Button::num5;
    keys.at(77) = win::Button::right;
    keys.at(78) = win::Button::num_plus;
    keys.at(79) = win::Button::end;
    keys.at(80) = win::Button::down;
    keys.at(81) = win::Button::page_down;
    keys.at(82) = win::Button::insert;
    keys.at(83) = win::Button::del;
    keys.at(87) = win::Button::f11;
    keys.at(88) = win::Button::f12;
    keys.at(91) = win::Button::lmeta;
    keys.at(92) = win::Button::rmeta;
}

}

#endif
