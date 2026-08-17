#pragma once

#include <win/MonitorEnumerator.hpp>
#include <win/Win.hpp>

namespace win
{

class Win32MonitorEnumerator : public MonitorEnumeratorBase
{
    WIN_NO_COPY_MOVE(Win32MonitorEnumerator);

public:
    Win32MonitorEnumerator();

    void refresh() override;
};

}
