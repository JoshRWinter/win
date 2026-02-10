#pragma once

#include <memory>
#include <vector>

#include <win/Win.hpp>

namespace win
{

struct Monitor
{
	Monitor(const std::string &id, bool primary, int x, int y, int width, int height, float rate)
		: id(id)
		, primary(primary)
		, x(x)
		, y(y)
		, width(width)
		, height(height)
		, rate(rate)
	{
	}

	std::string id;

	bool primary;
	int x;
	int y;
	int width;
	int height;
	float rate;
};

class MonitorEnumeratorBase
{
	WIN_NO_COPY_MOVE(MonitorEnumeratorBase);

protected:
	MonitorEnumeratorBase() = default;

public:
	virtual ~MonitorEnumeratorBase() = default;

	virtual void refresh() = 0;

	virtual int count() const = 0;
	virtual const Monitor &operator[](int index) const = 0;
	virtual std::vector<Monitor>::const_iterator begin() const = 0;
	virtual std::vector<Monitor>::const_iterator end() const = 0;
};

class MonitorEnumerator
{
	WIN_NO_COPY_MOVE(MonitorEnumerator);

public:
	MonitorEnumerator();

	void refresh();

	int count() const;
	const Monitor &operator[](int index) const;
	std::vector<Monitor>::const_iterator begin() const;
	std::vector<Monitor>::const_iterator end() const;

private:
	std::unique_ptr<MonitorEnumeratorBase> inner;
};

}
