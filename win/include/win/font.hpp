#pragma once

#include <array>

#include <win/win.hpp>
#include <win/stream.hpp>

namespace win
{

class FontRenderer;

struct metric
{
	float advance;
	float bearing_y;
	float bitmap_left;
};

class Font
{
	friend class FontRenderer;
public:
	static constexpr int rows = 6;
	static constexpr int cols = 16;

	Font(const FontRenderer &parent, Stream, float);
	~Font();

	WIN_NO_COPY_MOVE(Font);

	float size() const;

private:
	unsigned atlas;
	std::array<metric, 95> metrics;
	float box_width; // width of each tile in the atlas
	float box_height; // height of each tile in the atlas
	float max_bearing_y; // greatest y bearing
	float vertical; // vertical advance
	float fontsize;
};

}
