#pragma once

#include <array>
#include <memory>

#include <win/Win.hpp>
#include <win/Utility.hpp>
#include <win/Stream.hpp>

namespace win
{

struct FontCharacterMetric
{
	float width;
	float height;
	int width_pixels;
	int height_pixels;
	float advance;
	float bearing_y;
	float bearing_x;
};

struct FontMetric
{
	int max_width_pixels;
	int max_height_pixels;
	float vertical_advance;
};

class Font
{
	WIN_NO_COPY(Font);

public:
	static constexpr int char_count = ('~' - ' ') + 1;
	static constexpr char char_low = ' ';
	static constexpr char char_high = '~';

	Font(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area, float point_size, Stream font_file);
	Font(Font &&rhs) = default;

	Font &operator=(Font &&rhs) = default;

	const FontCharacterMetric &character_metric(char c) const;
	const FontMetric &font_metric() const;

protected:
	FontMetric metric;
	std::array<FontCharacterMetric, char_count> character_metrics;
	std::unique_ptr<unsigned char[]> bitmaps[char_count];

private:
	static void bitmap_copy(unsigned char *source, int source_width, int source_height, unsigned char *dest, int dest_width, int dest_height);
};

}
