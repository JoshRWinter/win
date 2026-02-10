#include <win/MonitorEnumerator.hpp>

#if defined WINPLAT_LINUX
#include <win/X11MonitorEnumerator.hpp>
#elif defined WINPLAT_WINDOWS
#include <win/Win32MonitorEnumerator.hpp>
#endif

namespace win
{

MonitorEnumerator::MonitorEnumerator()
{
#if defined WINPLAT_LINUX
	inner.reset(new X11MonitorEnumerator());
#elif defined WINPLAT_WINDOWS
	inner.reset(new Win32MonitorEnumerator());
#endif
}

void MonitorEnumerator::refresh()
{
	inner->refresh();
}

int MonitorEnumerator::count() const
{
	return inner->count();
}

const Monitor & MonitorEnumerator::operator[](int index) const
{
	return inner->operator[](index);
}

std::vector<Monitor>::const_iterator MonitorEnumerator::begin() const
{
	return inner->begin();
}

std::vector<Monitor>::const_iterator MonitorEnumerator::end() const
{
	return inner->end();
}

}
