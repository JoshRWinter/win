#include <string>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <win/win.hpp>
#include <win/font.hpp>
#include <win/fontrenderer.hpp>

#ifdef WIN_USE_OPENGL
#include <GL/gl.h>
#include <GL/glext.h>
#endif

namespace win
{

Font::Font(const FontRenderer &parent, Stream file, float size)
{
	fontsize = size;

	std::vector<unsigned char> chunk(file.size());
	file.read(chunk.data(), file.size());

	const int pixelsize = (fontsize / (parent.area.right - parent.area.left)) * parent.dims.width;

	int error;
	FT_Library library;
	error=FT_Init_FreeType(&library);
	if(error)
		win::bug("Error initializing freetype");

	FT_Face face;
	error=FT_New_Memory_Face(library,chunk.data(),chunk.size(),0,&face);
	if(error)
		win::bug("Error creating face");

	error=FT_Set_Pixel_Sizes(face,0,pixelsize);
	if(error)
		win::bug("Error setting pixel size");

	vertical = (((float)(face->size->metrics.height >> 6)) / parent.dims.width) * (parent.area.right - parent.area.left);

	// get largest width and height
	int bitmap_width = 0;
	int bitmap_height = 0;
	max_bearing_y = 0.0f;
	for(char x = ' '; x <= '~'; ++x)
	{
		error = FT_Load_Char(face, x, FT_LOAD_BITMAP_METRICS_ONLY);
		if(error)
			win::bug("Could not render glyph " + std::to_string(x));

		// fill in the metrics
		const int metric_index = x - ' ';
		metrics.at(metric_index).advance = ((float)(face->glyph->metrics.horiAdvance >> 6) / parent.dims.width) * (parent.area.right - parent.area.left);
		metrics[metric_index].bearing_y = (((float)((face->bbox.yMax / 2048.0f) * face->size->metrics.y_ppem) - (face->glyph->metrics.horiBearingY >> 6)) / parent.dims.height) * (parent.area.bottom - parent.area.top);
		metrics[metric_index].bitmap_left = ((float)face->glyph->bitmap_left / parent.dims.width) * (parent.area.right - parent.area.left);

		if(metrics[metric_index].bearing_y < max_bearing_y)
			max_bearing_y = metrics[metric_index].bearing_y;

		if((int)face->glyph->bitmap.width > bitmap_width)
			bitmap_width = (int)face->glyph->bitmap.width;
		if((int)face->glyph->bitmap.rows > bitmap_height)
			bitmap_height = (int)face->glyph->bitmap.rows;
	}

	box_width = ((float)bitmap_width / parent.dims.width) * (parent.area.right - parent.area.left);
	box_height = ((float)bitmap_height / parent.dims.height) * (parent.area.bottom - parent.area.top);

	std::vector<unsigned char> bitmap(bitmap_width * bitmap_height * rows * cols * 4);
	memset(bitmap.data(), 0, bitmap.size());
	for(unsigned char character = ' '; character <= '~'; character++)
	{
		error = FT_Load_Char(face,character,FT_LOAD_RENDER);
		if(error)
			win::bug(std::string("Error rendering char ") + std::to_string((int)character) + " (" + std::to_string(character) + ")");

		const unsigned char *buffer = face->glyph->bitmap.buffer;

		const int glyphwidth = face->glyph->bitmap.width;
		const int glyphheight = face->glyph->bitmap.rows;

		// where to render in the atlas bitmap
		const int xpos = ((character - 32) % cols) * bitmap_width;
		const int ypos = ((character - 32) / cols) * bitmap_height;
		const int index = ((bitmap_width * bitmap_height * rows * cols * 4) - (bitmap_width * cols * 4)) - (ypos * bitmap_width * cols * 4) + (xpos * 4);

		for(int i = index,j = 0;j < glyphwidth * glyphheight;)
		{
			if(i + 3 >= (int)bitmap.size() || i < 0)
			{
				char str[2] = {(char)character, 0};
				std::cerr << "char " << (char)character << " is " << glyphheight << " rows tall, max " << (((face->bbox.yMax - face->bbox.yMin) / 2048.0f) * face->size->metrics.y_ppem) << std::endl;
				win::bug("char " + std::to_string(character) + " (" + std::string(str) + ") out of bounds " + (i < 0 ? "negative" : "positive") + " by " + std::to_string(i < 0 ? -i : i - (bitmap_width * bitmap_height * rows * cols * 4)));
			}

			int level = buffer[j];
			if(level < 255 && level >= 0)
			{
				bitmap[i] = 255;
				bitmap[i + 1] = 255;
				bitmap[i + 2] = 255;
				bitmap[i + 3] = level;
			}
			else
			{
				bitmap[i] = 255;
				bitmap[i + 1] = 255;
				bitmap[i + 2] = 255;
				bitmap[i + 3] = 255;
			}

			j++;
			i += 4;

			if(j % glyphwidth == 0)
				i -= (bitmap_width * cols * 4) + (glyphwidth * 4);
		}
	}

	glGenTextures(1, &atlas);
	glBindTexture(GL_TEXTURE_2D, atlas);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bitmap_width * cols,bitmap_height * rows,0,GL_RGBA,GL_UNSIGNED_BYTE,bitmap.data());

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

Font::~Font()
{
	glDeleteTextures(1, &atlas);
}

float Font::size() const
{
	return fontsize;
}

}
