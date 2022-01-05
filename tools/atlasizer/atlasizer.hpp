#ifndef ATLASIZER_HPP
#define ATLASIZER_HPP

#include <list>
#include <memory>
#include <cmath>
#include <string>

#include <GL/gl.h>

#include "targa.hpp"

class AtlasItem
{
public:
	AtlasItem(const std::string &filepath, int x, int y)
		: filename(filepath)
		, targa(filepath)
		, x(x)
		, y(y)
	{
		width = targa.width();
		height = targa.height();
	}

	bool oob(int padding) const
	{
		return x < padding || y < padding;
	}

	void correct_bounds(int padding)
	{
		if (x < padding)
			x = padding;
		if (y < padding)
			y = padding;
	}

	bool colliding(const AtlasItem &rhs, int padding) const
	{
		return x + width + padding > rhs.x && x < rhs.x + rhs.width + padding && y + height + padding > rhs.y && y < rhs.y + rhs.height + padding;
	}

	void correct(const AtlasItem &item, int padding)
	{
		if (!colliding(item, padding))
			return;

		const int ldiff = std::abs(x - (item.x + item.width + padding));
		const int rdiff = std::abs((x + width) - (item.x + padding));
		const int tdiff = std::abs((y + height) - (item.y + padding));
		const int bdiff = std::abs(y - (item.y + item.height + padding));

		int smallest = ldiff;
		if (rdiff < smallest) smallest = rdiff;
		if (tdiff < smallest) smallest = tdiff;
		if (bdiff < smallest) smallest = bdiff;

		if (smallest == ldiff)
			x = item.x + item.width + padding;
		else if (smallest == rdiff)
			x = item.x - width - padding;
		else if (smallest == tdiff)
			y = item.y - height - padding;
		else if (smallest == bdiff)
			y = item.y + item.height + padding;
	}

public:
	std::string filename;
	Targa targa;
	int x;
	int y;
	int width;
	int height;
};

void gui();

#endif
