#pragma once

#include <memory>

#include <win/Win.hpp>
#include <win/Stream.hpp>

namespace win
{

struct AtlasItem
{
	float x1;
	float y1;
	float x2;
	float y2;
};

class Atlas
{
	WIN_NO_COPY_MOVE(Atlas);

public:
	explicit Atlas(Stream);

	int count() const;
	const AtlasItem &item(int) const;

	int width() const;
	int height() const;
	const unsigned char *data() const;


private:
	static void corrupt();

	int num;
	std::unique_ptr<AtlasItem[]> textures;

	int canvas_width;
	int canvas_height;
	std::unique_ptr<unsigned char[]> imgdata;
};

}
