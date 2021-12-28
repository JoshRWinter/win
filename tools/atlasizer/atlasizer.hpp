#ifndef ATLASIZER_HPP
#define ATLASIZER_HPP

#include <list>
#include <memory>
#include <cmath>
#include <string>

#include "targa.hpp"

struct AtlasItemDescriptor
{
	std::string filename;
	int x;
	int y;
	int width;
	int height;
};

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

public:
	std::string filename;
	Targa targa;
	int x;
	int y;
	int width;
	int height;
};

void gui();
void compileatlas(const std::string&, const std::string&);

#endif
