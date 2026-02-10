#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <cmath>

#include <dxgi.h>

#include <win/Win32MonitorEnumerator.hpp>

namespace win
{

struct Win32MonitorInfo
{
	std::string name;
	int width;
	int height;
	int rate;
	bool primary;
};

static BOOL __stdcall callback(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM lp)
{
	std::vector<Win32MonitorInfo>& v = *(std::vector<Win32MonitorInfo>*)lp;

	MONITORINFOEX info;
	info.cbSize = sizeof(MONITORINFOEX);

	if (!GetMonitorInfo(monitor, &info))
		win::bug("GetMonitorInfo failed");

	DEVMODE dm;
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &dm))
	{
		win::bug("EnumDisplaySettings failed");
	}

	auto &item = v.emplace_back();
	item.name = info.szDevice;
	item.width = dm.dmPelsWidth;
	item.height = dm.dmPelsHeight;
	item.rate = dm.dmDisplayFrequency;
	item.primary = info.dwFlags & MONITORINFOF_PRIMARY;

	return TRUE;
}

Win32MonitorEnumerator::Win32MonitorEnumerator()
{
	Win32MonitorEnumerator::refresh();
}

void Win32MonitorEnumerator::refresh()
{
	monitors.clear();

	IDXGIFactory1 *factory;
	HRESULT result;

	std::vector<DXGI_MODE_DESC> modes;

	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);

	if (result != S_OK)
	{
		fprintf(stderr, "CreateDXGIFactory1 failed: %ld\n", result);
		return;
	}

	std::vector<Win32MonitorInfo> win32_monitors;;
	EnumDisplayMonitors(NULL, NULL, callback, (LPARAM)&win32_monitors);

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

			// Turrible.
			static_assert(sizeof(desc.DeviceName) == sizeof(WCHAR) * 32, "DXGI_OUTPUT_DESC::DeviceName must be 32 WCHAR array");
			char name[33];
			for (int j = 0; j < 32; ++j)
				name[j] = desc.DeviceName[j];
			name[32] = 0;

			// find the matching win32 monitor
			const Win32MonitorInfo *win32mon = NULL;
			for (const auto &m : win32_monitors)
			{
				if (!strcmp(m.name.c_str(), name))
				{
					win32mon = &m;
					break;
				}
			}

			if (win32mon == NULL)
				win::bug("Couldn't match DXGI monitor " + std::string(name) + " to a WIN32 monitor");

			const bool primary = win32mon->primary;

			const DXGI_MODE_DESC *closestmode = NULL;
			// These modes are all the possible modes for the monitor. But we have to figure out which mode the user is currently using.
			for (const auto &mode : modes)
			{
				if (mode.Width == win32mon->width && mode.Height == win32mon->height)
				{
					if (closestmode == NULL)
						closestmode = &mode;
					else
					{
						const float rate = mode.RefreshRate.Numerator / (float)mode.RefreshRate.Denominator;
						const float diff = std::fabs(win32mon->rate - rate);

						const float rate2 = closestmode->RefreshRate.Numerator / (float)closestmode->RefreshRate.Denominator;
						const auto diff2 = std::fabs(win32mon->rate - rate2);

						if (diff < diff2)
							closestmode = &mode;
					}
				}
			}

			if (closestmode == NULL)
				win::bug("Couldn't determine active mode for monitor " + win32mon->name);

			monitors.emplace_back(name, primary, desc.DesktopCoordinates.left, desc.DesktopCoordinates.top, closestmode->Width, closestmode->Height, closestmode->RefreshRate.Numerator / (float)closestmode->RefreshRate.Denominator);

			output->Release();
			++i2;
		}

		adapter->Release();
		++i;
	}

	factory->Release();
}

int Win32MonitorEnumerator::count() const
{
	return monitors.size();
}

const Monitor &Win32MonitorEnumerator::operator[](int index) const
{
	return monitors.at(index);
}

std::vector<Monitor>::const_iterator Win32MonitorEnumerator::begin() const
{
	return monitors.begin();
}

std::vector<Monitor>::const_iterator Win32MonitorEnumerator::end() const
{
	return monitors.end();
}

}

#endif
