#include <cstring>

#include <win/gl/GLFont.hpp>

using namespace win::gl;

namespace win
{

GLFont::GLFont(const win::Dimensions<int> &screen_pixel_dimensions, const win::Area<float> &screen_area, const float font_size, Stream font_file)
	: Font(screen_pixel_dimensions, screen_area, font_size, std::move(font_file))
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex.get());
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, metric.max_width_pixels, metric.max_height_pixels, Font::char_count, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	std::unique_ptr<unsigned char[]> flipped(new unsigned char[metric.max_width_pixels * metric.max_height_pixels]);
	for (int i = 0; i < char_count; ++i)
	{
		const FontCharacterMetric &cmetric = character_metric(i + char_low);
		memset(flipped.get(), 0, metric.max_width_pixels * metric.max_height_pixels);

		for (int source_row = 0; source_row < cmetric.height_pixels; ++source_row)
		{
			const int dest_row = (cmetric.height_pixels - 1) - source_row;

			const unsigned char *const source = bitmaps[i].get() + (metric.max_width_pixels * source_row);
			unsigned char *const dest = flipped.get() + (metric.max_width_pixels * dest_row);

			memcpy(dest, source, cmetric.width_pixels);
		}

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, metric.max_width_pixels, metric.max_height_pixels, 1, GL_RED, GL_UNSIGNED_BYTE, flipped.get());
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// free up some memory
	for (int i = 0; i < char_count; ++i)
		bitmaps[i].reset();
}

}
