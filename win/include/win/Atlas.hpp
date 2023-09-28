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
	WIN_NO_COPY(Atlas);

public:
	explicit Atlas(Stream);
	Atlas(Atlas&&) = default;

	int count() const;
	const AtlasItem &item(int) const;

protected:
	int canvas_width;
	int canvas_height;
	std::unique_ptr<unsigned char[]> imgdata;

private:
	static void corrupt();

	int num;
	std::unique_ptr<AtlasItem[]> textures;
};

}
