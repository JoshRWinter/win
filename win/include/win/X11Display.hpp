#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <chrono>

#include <GL/glx.h>
#include <X11/Xlib.h>

#include <win/DisplayBase.hpp>

namespace win
{

class X11Display : public DisplayBase
{
	WIN_NO_COPY_MOVE(X11Display);

public:
	explicit X11Display(const DisplayOptions &options);
	~X11Display() override;

	void process() override;
	void swap() override;
	int width() override;
	int height() override;
	void resize(int w, int h) override;
	float refresh_rate() override;
	void cursor(bool show) override;
	void vsync(bool on) override;
	void set_fullscreen(bool fullscreen) override;
	NativeWindowHandle native_handle() override;

private:
	void update_refresh_rate();
	void get_current_monitor_props(int &x, int &y, int &w, int &h, float &rr);
	static bool contains_point(int monitorx, int monitory, int monitorw, int monitorh, int x, int y);

	const DisplayOptions options;
	Window window;
	GLXContext context;
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;

	struct
	{
		int x = 0, y = 0, w = 0, h = 0;
		float rate = 60.0f;
	} mon_props_cache;

	struct
	{
		int x = 0, y = 0, w = 0, h = 0;
	} window_prop_cache;

	struct
	{
		bool resized = false;
		std::chrono::time_point<std::chrono::steady_clock> time;
	} resize_state;

	float rrate = 60;
};

}

#endif
