#pragma once

#include <win/Win.hpp>
#include <win/MonitorEnumerator.hpp>

namespace win
{

class Win32MonitorEnumerator : public MonitorEnumeratorBase
{
	WIN_NO_COPY_MOVE(Win32MonitorEnumerator);

public:
	Win32MonitorEnumerator();

	void refresh() override;

	int count() const override;
	const Monitor &operator[](int index) const override;
	std::vector<Monitor>::const_iterator begin() const override;
	std::vector<Monitor>::const_iterator end() const override;

private:
	std::vector<Monitor> monitors;
};

}
