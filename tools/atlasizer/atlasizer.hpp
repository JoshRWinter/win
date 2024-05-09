#pragma once

#include <list>
#include <memory>
#include <cmath>
#include <string>

#include <win/FileReadStream.hpp>
#include <win/Targa.hpp>

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
		, targa(win::Stream(new win::FileReadStream(filepath)))
		, x(x)
		, y(y)
	{
		bpp = targa.bpp();
		width = targa.width();
		height = targa.height();
	}

public:
	std::string filename;
	win::Targa targa;
	int x;
	int y;
	int bpp;
	int width;
	int height;
};

void gui();
void compileatlas(const std::string&, const std::string&);
std::unique_ptr<unsigned char[]> convert_to_bgra8(const win::Targa &targa);
