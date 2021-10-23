#include <ft2build.h>
#include FT_FREETYPE_H

#include <memory>
#include <climits>

#include <math.h>
#include <string.h>

#include <win.h>

static constexpr int cols = 16;
static constexpr int rows = 6;

namespace win
{

Font::Font(const FontRenderer &parent, AssetRollStream file, float size)
{
	fontsize = size;

	std::vector<unsigned char> chunk(file.size());
	file.read(chunk.data(), file.size());

	const int pixelsize = (fontsize / (parent.right - parent.left)) * parent.display_width;

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

	vertical = (((float)(face->size->metrics.height >> 6)) / parent.display_width) * (parent.right - parent.left);

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
		metrics.at(metric_index).advance = ((float)(face->glyph->metrics.horiAdvance >> 6) / parent.display_width) * (parent.right - parent.left);
		metrics[metric_index].bearing_y = (((float)((face->bbox.yMax / 2048.0f) * face->size->metrics.y_ppem) - (face->glyph->metrics.horiBearingY >> 6)) / parent.display_height) * (parent.bottom - parent.top);
		metrics[metric_index].bitmap_left = ((float)face->glyph->bitmap_left / parent.display_width) * (parent.right - parent.left);

		if(metrics[metric_index].bearing_y < max_bearing_y)
			max_bearing_y = metrics[metric_index].bearing_y;

		if((int)face->glyph->bitmap.width > bitmap_width)
			bitmap_width = (int)face->glyph->bitmap.width;
		if((int)face->glyph->bitmap.rows > bitmap_height)
			bitmap_height = (int)face->glyph->bitmap.rows;
	}

	box_width = ((float)bitmap_width / parent.display_width) * (parent.right - parent.left);
	box_height = ((float)bitmap_height / parent.display_height) * (parent.bottom - parent.top);

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

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bitmap_width * cols,bitmap_height * rows,0,GL_RGBA,GL_UNSIGNED_BYTE,bitmap.data());

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	atlas = Texture(tex);
}

float Font::size() const
{
	return fontsize;
}

static inline float alignx(int iwidth, float fwidth, float xpos)
{
	return ((((int)((xpos / fwidth) * iwidth)) / (float)iwidth) * fwidth);
}

static inline float aligny(int iheight, float fheight, float ypos)
{
	return ((((int)((ypos / fheight) * iheight)) / (float)iheight) * fheight);
}

// font shaders
static const char *vertexshader =
"#version 330 core\n"
"layout (location = 0) in vec2 vert;\n"
"layout (location = 1) in vec2 texcoord_offset;\n"
"layout (location = 2) in vec2 pos;\n"
"layout (location = 3) in vec2 texcoord;\n"
"uniform mat4 projection;\n"
"uniform vec2 size;\n"
"out vec2 ftexcoord;\n"
"void main(){\n"
"ftexcoord = vec2(texcoord.x + texcoord_offset.x, texcoord.y - texcoord_offset.y);\n"
"mat4 translate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x + (size.x / 2.0), pos.y + (size.y / 2.0), 0.0, 1.0);\n"
"gl_Position = projection * translate * vec4(vert.x * size.x, vert.y * size.y, 0.0, 1.0);\n"
"}\n"
,*fragmentshader =
"#version 330 core\n"
"out vec4 pixel;\n"
"in vec2 ftexcoord;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"void main(){\n"
"pixel = texture(tex, ftexcoord) * color;\n"
"}\n"
;

FontRenderer::FontRenderer(int iwidth, int iheight, float left, float right, float bottom, float top)
{
	display_width = iwidth;
	display_height = iheight;
	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;

	// shaders and uniforms
	program = Program(load_shaders(vertexshader, fragmentshader));
	glUseProgram(program.get());
	uniform_size = glGetUniformLocation(program.get(), "size");
	uniform_color = glGetUniformLocation(program.get(), "color");
	int uniform_projection = glGetUniformLocation(program.get(), "projection");

	float ortho_matrix[16];
	win::init_ortho(ortho_matrix, left, right, bottom, top);
	glUniformMatrix4fv(uniform_projection, 1, false, ortho_matrix);

	glBindVertexArray(vao.get());

	// element indices
	unsigned int indices[] =
	{
		0, 1, 3, 3, 1, 2
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// font vertices
	const float verts[] =
	{
		-0.5f, 0.5f, 0.0f, (1.0f / rows),
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, (1.0f / cols), 0.0f,
		0.5f, 0.5f, (1.0f / cols), (1.0f / rows)
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex.get());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	// generic attributes
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position.get());
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord.get());
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, true, 0, NULL);
}

void FontRenderer::draw(const Font &fnt, const char *text, float xpos, float ypos, const Color &clr, bool centered)
{
	const int textlen = strlen(text);

	pos_buffer.clear();
	texcoord_buffer.clear();

	// fill vbos
	float xoffset;
	if(centered)
		xoffset = xpos - (line_length(fnt, text, 0) / 2.0f);
	else
		xoffset = xpos;
	float yoffset = ypos - fnt.max_bearing_y;
	int charcount = 0;
	for(int i = 0; i < textlen; ++i)
	{
		const int metrics_index = text[i] - ' ';

		if(text[i] == '\n')
		{
			yoffset += fnt.vertical;
			if(centered)
				xoffset = xpos - (line_length(fnt, text, i + 1) / 2.0f);
			else
				xoffset = xpos;
			continue;
		}
		else if(text[i] == ' ')
		{
			xoffset += fnt.metrics.at(metrics_index).advance - fnt.metrics.at(metrics_index).bitmap_left;
			continue;
		}

		if(text[i] < ' ' || text[i] > '~')
			win::bug("non printing ascii character: " + std::to_string((int)text[i]) + " found in text string");

		// pos vbo
		pos_buffer.push_back(alignx(display_width, right - left, xoffset));
		pos_buffer.push_back(aligny(display_height, top - bottom, yoffset + fnt.metrics.at(metrics_index).bearing_y));

		// texcoord vbo
		const float xnormal = 1.0f / cols;
		const float ynormal = 1.0f / rows;
		unsigned short xcoord = ((float)((text[i] - ' ') % 16) * xnormal) * USHRT_MAX;
		unsigned short ycoord = ((float)((text[i] - ' ') / 16) * ynormal) * USHRT_MAX;
		texcoord_buffer.push_back(xcoord);
		texcoord_buffer.push_back(USHRT_MAX - ycoord);

		xoffset += fnt.metrics[metrics_index].advance - fnt.metrics.at(metrics_index).bitmap_left;

		++charcount;
	}

	glBindVertexArray(vao.get());
	glUseProgram(program.get());

	glUniform2f(uniform_size, fnt.box_width, fnt.box_height);
	glUniform4f(uniform_color, clr.red, clr.green, clr.blue, clr.alpha);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position.get());
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * charcount * 2, pos_buffer.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord.get());
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned short) * 2 * charcount, texcoord_buffer.data(), GL_DYNAMIC_DRAW);

	glBindTexture(GL_TEXTURE_2D, fnt.atlas.get());

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, charcount);
}

// calculate line length, only up to the first newline after <start>
float FontRenderer::line_length(const Font &fnt, const char *text, int start) const
{
	const int len = strlen(text);
	float length = 0.0f;

	for(int i = start; i < len; ++i)
	{
		if(text[i] == '\n')
			break;

		length += fnt.metrics[text[i] - ' '].advance - fnt.metrics[text[i] - ' '].bitmap_left;
	}

	return length;
}

}
