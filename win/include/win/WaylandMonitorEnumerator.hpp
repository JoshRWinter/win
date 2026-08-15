#pragma once

#include <vector>

#include <win/MonitorEnumerator.hpp>

namespace win
{

class WaylandMonitorEnumerator : public MonitorEnumeratorBase
{
    WIN_NO_COPY_MOVE(WaylandMonitorEnumerator);

public:
    WaylandMonitorEnumerator();

    void refresh() override;

private:
    void init();
};

}
