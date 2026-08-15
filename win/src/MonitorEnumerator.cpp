#include <win/MonitorEnumerator.hpp>

#if defined WINPLAT_LINUX
#include <win/WaylandMonitorEnumerator.hpp>
#elif defined WINPLAT_WINDOWS
#include <win/Win32MonitorEnumerator.hpp>
#endif

namespace win
{

int MonitorEnumeratorBase::count()
{
    return monitors.size();
}

std::vector<Monitor>::const_iterator MonitorEnumeratorBase::begin()
{
    return monitors.begin();
}

std::vector<Monitor>::const_iterator MonitorEnumeratorBase::end()
{
    return monitors.end();
}

MonitorEnumerator::MonitorEnumerator()
{
#if defined WINPLAT_LINUX
    inner.reset(new WaylandMonitorEnumerator());
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

std::vector<Monitor>::const_iterator MonitorEnumerator::begin() const
{
    return inner->begin();
}

std::vector<Monitor>::const_iterator MonitorEnumerator::end() const
{
    return inner->end();
}

}
