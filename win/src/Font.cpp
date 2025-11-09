#include <string>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <win/Font.hpp>

namespace win
{

Font::Font(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area, const float font_size, Stream font_file)
{
	auto data = font_file.read_all();

	const int pixelsize = std::roundf((font_size / (screen_area.right - screen_area.left)) * screen_pixel_dimensions.width);

	int error;
	FT_Library library;
	error = FT_Init_FreeType(&library);
	if (error)
		win::bug("Error initializing freetype");

	FT_Face face;
	error = FT_New_Memory_Face(library, data.get(), font_file.size(), 0, &face);
	if (error)
		win::bug("Font: Error creating face");

	error = FT_Set_Pixel_Sizes(face, 0, pixelsize);
	if (error)
		win::bug("Error setting pixel size");

	metric.vertical_advance = ((face->size->metrics.height >> 6) / (float)screen_pixel_dimensions.height) * (screen_area.top - screen_area.bottom);

	FT_UInt indices[char_count];
	// get largest width and height
	metric.max_width_pixels = 0;
	metric.max_height_pixels = 0;
	for (char x = char_low; x <= char_high; ++x) // loop over the printing ascii characters
	{
		error = FT_Load_Char(face, x, FT_LOAD_BITMAP_METRICS_ONLY);
		if (error)
			win::bug("Could not render glyph " + std::to_string(x));

		indices[x - char_low] = face->glyph->glyph_index;

		if ((int)face->glyph->bitmap.width > metric.max_width_pixels)
			metric.max_width_pixels = (int)face->glyph->bitmap.width;
		if ((int)face->glyph->bitmap.rows > metric.max_height_pixels)
			metric.max_height_pixels = (int)face->glyph->bitmap.rows;
	}

	metric.max_width = (metric.max_width_pixels / (float)screen_pixel_dimensions.width) * (screen_area.right - screen_area.left);
	metric.max_height = (metric.max_height_pixels / (float)screen_pixel_dimensions.height) * (screen_area.top - screen_area.bottom);

	for (char character = char_low; character <= char_high; character++)
	{
		bitmaps[character - char_low].reset(new unsigned char[metric.max_width_pixels * metric.max_height_pixels]);
		unsigned char *const bitmap = bitmaps[character - char_low].get();
		memset(bitmap, 0, metric.max_width_pixels * metric.max_height_pixels);

		error = FT_Load_Char(face, character, FT_LOAD_RENDER);
		if (error)
			win::bug(std::string("Error rendering char ") + std::to_string((int)character) + " (" + std::to_string(character) + ")");

		const int width_pixels = face->glyph->bitmap.width;//(face->glyph->metrics.width / (float)face->units_per_EM) * face->size->metrics.x_ppem;
		const int height_pixels = face->glyph->bitmap.rows;//(face->glyph->metrics.height / (float)face->units_per_EM) * face->size->metrics.y_ppem;

		// fill in the metrics
		const int metric_index = character - char_low;
		FontCharacterMetric &cmetric = character_metrics.at(metric_index);

		cmetric.width = (width_pixels / (float)screen_pixel_dimensions.width) * (screen_area.right - screen_area.left);
		cmetric.height = (height_pixels / (float)screen_pixel_dimensions.height) * (screen_area.top - screen_area.bottom);
		cmetric.width_pixels = width_pixels;
		cmetric.height_pixels = height_pixels;
		cmetric.advance = ((float)(face->glyph->metrics.horiAdvance >> 6) / screen_pixel_dimensions.width) * (screen_area.right - screen_area.left);
		cmetric.bearing_y = (((face->glyph->metrics.horiBearingY >> 6) / (float)screen_pixel_dimensions.height)) * (screen_area.top - screen_area.bottom);
		cmetric.bearing_x = (((face->glyph->metrics.horiBearingX >> 6) / (float)screen_pixel_dimensions.width)) * (screen_area.right - screen_area.left);

		bitmap_copy(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, bitmap, metric.max_width_pixels, metric.max_height_pixels);

		// figure out them kerns
		for (int k = char_low; k < char_high; ++k)
		{
			if (!FT_HAS_KERNING(face))
				continue;

			FT_Vector v;
			error = FT_Get_Kerning(face, face->glyph->glyph_index, indices[k - char_low], FT_KERNING_DEFAULT, &v);
			if (error)
				win::bug("No kern for " + std::to_string((char)character) + " vs " + std::to_string((char)k));

			if (v.x != 0)
				cmetric.kerns.emplace_back(k, ((v.x >> 6) / (float)screen_pixel_dimensions.width) * (screen_area.right - screen_area.left));
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

const FontMetric &Font::font_metric() const
{
	return metric;
}

const FontCharacterMetric &Font::character_metric(const char c) const
{
#ifndef NDEBUG
	return character_metrics.at(c - char_low);
#endif

	return character_metrics[c - char_low];
}

void Font::bitmap_copy(unsigned char *source, int source_width, int source_height, unsigned char *dest, int dest_width, int dest_height)
{
	if (source_width > dest_width)
		win::bug("Font::bitmap_copy: source_width > dest_width");

	if (source_height > dest_height)
		win::bug("Font::bitmap_copy: source_height > dest_height");

	for (int source_row = 0; source_row < source_height; ++source_row)
	{
		const int source_start = source_row * source_width;
		const int dest_start = source_row * dest_width;
		memcpy(dest + dest_start, source + source_start, source_width);
	}
}

}
