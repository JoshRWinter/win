#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <vector>

#include <gl/GL.h>
#include <GL/wglext.h>

#include <win/DisplayBase.hpp>

namespace win
{

class Win32Display : public DisplayBase
{
	WIN_NO_COPY_MOVE(Win32Display);

	constexpr static DWORD windowed_style = WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION | WS_SIZEBOX;
	constexpr static DWORD fullscreen_style = WS_POPUP;

	struct MonitorProperties
	{
		std::string name;
		int width;
		int height;
		float rate;
	};

public:
	explicit Win32Display(const DisplayOptions &options);
	~Win32Display() override;

	void process() override;
	void swap() override;
	int width() override;
	int height() override;
	void resize(int w, int h) override;
	float refresh_rate() override;
	void cursor(bool show) override;
	void set_fullscreen(bool fullscreen) override;
	void vsync(bool on) override;
	NativeWindowHandle native_handle() override;

private:
	static LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	void win_init_gl(HWND);
	void win_term_gl();
	void init_monitor_properties();
	void update_refresh_rate();

	HWND window;
	HDC hdc;
	HGLRC context;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	std::vector<MonitorProperties> monprops;
	float rrate;
	const win::DisplayOptions options;
	struct { int w = 0, h = 0; } window_prop_cache;
};

}

#endif
