#include <ft2build.h>
#include FT_FREETYPE_H

#include <math.h>

#include "win.h"

static constexpr int cols = 16;
static constexpr int rows = 6;

#define isvalidchar(c) ((c>31)||c=='\n'||c=='\r')

win::font::font(display &parent, resource &rc, float fontsize, int idisplay_width, int idisplay_height, float fdisplay_width, float fdisplay_height)
	: parent_(parent)
{
	std::vector<unsigned char> chunk = rc.read();

	const int pixelsize = (fontsize / fdisplay_width) * idisplay_width;

	int error;
	FT_Library library;
	error=FT_Init_FreeType(&library);
	if(error)
		throw exception("Error initializing freetype");

	FT_Face face;
	error=FT_New_Memory_Face(library,chunk.data(),chunk.size(),0,&face);
	if(error)
		throw exception("Error creating face");

	error=FT_Set_Pixel_Sizes(face,0,pixelsize);
	if(error){
		throw exception("Error setting pixel size");
	}

	const int adjustedsize = roundf(((face->bbox.yMax - face->bbox.yMin) / 2048.0f) * face->size->metrics.y_ppem);
	size_ = ((float)adjustedsize/idisplay_width) * fdisplay_width;

	std::vector<unsigned char> bitmap(adjustedsize * adjustedsize * rows * cols * 4);
	for(unsigned char character = 32;character < 127;character++){
		error = FT_Load_Char(face,character,FT_LOAD_RENDER);
		if(error)
			throw exception(std::string("Error rendering char ") + std::to_string((int)character) + " (" + std::to_string(character) + ")");

		const int glyphwidth = face->glyph->bitmap.width,glyphheight = face->glyph->bitmap.rows;
		kern_.at(character - 32).advance = ((face->glyph->metrics./*metrics.horiAdvance>>6*/horiAdvance >> 6)/(float)idisplay_width) * fdisplay_width;
		kern_.at(character - 32).bitmap_left = (face->glyph->bitmap_left / (float)idisplay_width) * fdisplay_width;
		const unsigned char *buffer = face->glyph->bitmap.buffer;
		const int xpos = ((character - 32) % cols) * adjustedsize;
		const int bearingadjust = ((face->bbox.yMax / 2048.0f) * face->size->metrics.y_ppem) - (face->glyph->metrics.horiBearingY >> 6);
		const int ypos = (((character - 32) / cols) * adjustedsize) + bearingadjust;
		const int index = ((adjustedsize * adjustedsize * rows * cols * 4) - (adjustedsize * cols * 4)) - (ypos * adjustedsize * cols * 4) + (xpos * 4);

		for(int i = index,j = 0;j < glyphwidth * glyphheight;){
			if(i + 3 >= adjustedsize * adjustedsize * rows * cols * 4 || i < 0)
				throw exception("char " + std::to_string(character) + " out of bounds " + (i < 0 ? "negative" : "positive") + " by " + std::to_string(i < 0 ? -i : i - (adjustedsize * adjustedsize * rows * cols * 4)));

			int level = buffer[j];
			if(level < 255 && level > 0){
				bitmap[i] = 255;
				bitmap[i + 1] = 255;
				bitmap[i + 2] = 255;
				bitmap[i + 3] = level;
			}
			else if(level == 0){
				bitmap[i] = 0;
				bitmap[i + 1] = 0;
				bitmap[i + 2] = 0;
				bitmap[i + 3] = 0;
			}
			else{
				bitmap[i] = 255;
				bitmap[i + 1] = 255;
				bitmap[i + 2] = 255;
				bitmap[i + 3] = 255;
			}

			j++;
			i += 4;

			if(j % glyphwidth == 0){
				i -= (adjustedsize * cols * 4) + (glyphwidth * 4);
			}
		}
	}

	glGenTextures(1, &atlas_);
	glBindTexture(GL_TEXTURE_2D, atlas_);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,adjustedsize * cols,adjustedsize * rows,0,GL_RGBA,GL_UNSIGNED_BYTE,bitmap.data());

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	unsigned short header[9]={
		0x0000,
		0x0002,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		(unsigned short)(adjustedsize * cols),
		(unsigned short)(adjustedsize * rows),
		0x0820
	};

	std::ofstream out("/home/josh/atlas.tga", std::ofstream::binary);
	out.write((char*)header, sizeof(header));
	out.write((char*)bitmap.data(), bitmap.size());
}

win::font::font(font &&rhs)
	: parent_(rhs.parent_)
{
	atlas_ = rhs.atlas_;
	kern_ = rhs.kern_;
	size_ = rhs.size_;

	rhs.atlas_ = 0;
}

win::font::~font()
{
	if(atlas_ != 0)
		glDeleteTextures(1, &atlas_);
}
