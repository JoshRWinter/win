#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

#include <win/X11MonitorEnumerator.hpp>

namespace win
{

X11MonitorEnumerator::X11MonitorEnumerator()
{
	X11MonitorEnumerator::refresh();
}

void X11MonitorEnumerator::refresh()
{
	monitors.clear();

	auto xdisplay = XOpenDisplay(NULL);
	auto resources = XRRGetScreenResources(xdisplay, DefaultRootWindow(xdisplay));

	for (int i = 0; i < resources->noutput; ++i)
	{
		auto info = XRRGetOutputInfo(xdisplay, resources, resources->outputs[i]);
		auto crtc = XRRGetCrtcInfo(xdisplay, resources, info->crtc);
		XRRModeInfo *mode = NULL;
		for (int j = 0; j < resources->nmode; ++j)
		{
			if (resources->modes[j].id == crtc->mode)
			{
				mode = &resources->modes[j];
				break;
			}
		}

		if (mode == NULL)
			win::bug("Couldn't find mode " + std::to_string(crtc->mode) + " for monitor " + info->name);

		monitors.emplace_back(info->name, false, crtc->x, crtc->y, crtc->width, crtc->height, mode->dotClock / ((double)mode->hTotal * mode->vTotal));

		XRRFreeCrtcInfo(crtc);
		XRRFreeOutputInfo(info);
	}

	XRRFreeScreenResources(resources);
	XCloseDisplay(xdisplay);

	if (!monitors.empty())
		monitors.at(0).primary = true;
}

int X11MonitorEnumerator::count() const
{
	return monitors.size();
}

const Monitor &X11MonitorEnumerator::operator[](int index) const
{
	return monitors.at(index);
}

std::vector<Monitor>::const_iterator X11MonitorEnumerator::begin() const
{
	return monitors.begin();
}

std::vector<Monitor>::const_iterator X11MonitorEnumerator::end() const
{
	return monitors.end();
}

}

#endif
