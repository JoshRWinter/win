#pragma once

#include <win/MonitorEnumerator.hpp>

namespace win
{

class X11MonitorEnumerator : public MonitorEnumeratorBase
{
	WIN_NO_COPY_MOVE(X11MonitorEnumerator);

public:
	X11MonitorEnumerator();

	void refresh() override;

	int count() const override;
	const Monitor &operator[](int index) const override;
	std::vector<Monitor>::const_iterator begin() const override;
	std::vector<Monitor>::const_iterator end() const override;

private:
	std::vector<Monitor> monitors;
};

}
