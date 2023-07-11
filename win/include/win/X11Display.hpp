#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <X11/Xlib.h>
#include <GL/glx.h>

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
	int screen_width() override;
	int screen_height() override;
	void cursor(bool show) override;
	void vsync(bool on) override;
	NativeWindowHandle native_handle() override;

private:
	Window window;
	GLXContext context;
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
};

}

#endif
