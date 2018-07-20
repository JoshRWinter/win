#include <ft2build.h>
#include FT_FREETYPE_H

#include <memory>
#include <climits>

#include <math.h>
#include <string.h>

#include "win.h"

static constexpr int cols = 16;
static constexpr int rows = 6;

#define isvalidchar(c) ((c>31)||c=='\n'||c=='\r')

win::font::font(const font_renderer &parent, resource &rc, float fontsize)
	: parent_(&parent)
{
	std::vector<unsigned char> chunk = rc.read();

	const int pixelsize = (fontsize / (parent.right_ - parent.left_)) * parent.display_width_;

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
	size_ = ((float)adjustedsize / parent.display_width_) * (parent.right_ - parent.left_);

	std::vector<unsigned char> bitmap(adjustedsize * adjustedsize * rows * cols * 4);
	for(unsigned char character = 32;character < 127;character++){
		error = FT_Load_Char(face,character,FT_LOAD_RENDER);
		if(error)
			throw exception(std::string("Error rendering char ") + std::to_string((int)character) + " (" + std::to_string(character) + ")");

		const int glyphwidth = face->glyph->bitmap.width,glyphheight = face->glyph->bitmap.rows;
		kern_.at(character - 32).advance = ((face->glyph->metrics./*metrics.horiAdvance>>6*/horiAdvance >> 6)/(float)parent.display_width_) * (parent.right_ - parent.left_);
		kern_.at(character - 32).bitmap_left = (face->glyph->bitmap_left / (float)parent.display_width_) * (parent.right_ - parent.left_);
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
}

win::font::font(font &&rhs)
{
	atlas_ = rhs.atlas_;
	kern_ = rhs.kern_;
	parent_ = rhs.parent_;
	size_ = rhs.size_;

	rhs.atlas_ = 0;
}

win::font::~font()
{
	finalize();
}

win::font &win::font::operator=(font &&rhs)
{
	finalize();

	atlas_ = rhs.atlas_;
	kern_ = rhs.kern_;
	parent_ = rhs.parent_;
	size_ = rhs.size_;

	atlas_ = 0;

	return *this;
}

static inline float alignx(int iwidth, float fwidth, float xpos)
{
	return ((((int)((xpos / fwidth) * iwidth)) / (float)iwidth) * fwidth);
}

static inline float aligny(int iheight, float fheight, float ypos)
{
	return ((((int)((ypos / fheight) * iheight)) / (float)iheight) * fheight);
}

void win::font::draw(const char *text, float xpos, float ypos, float r, float g, float b, float a)
{
	const int textlen = strlen(text);

	std::unique_ptr<float[]> pos_buffer(new float[2 * textlen]);
	std::unique_ptr<unsigned short[]> texcoord_buffer(new unsigned short[2 * textlen]);

	// fill vbos
	float xoffset = xpos;
	float yoffset = ypos;
	for(int i = 0; i < textlen; ++i)
	{
		if(text[i] == '\n')
		{
			yoffset += size_;
			xoffset = xpos;
			continue;
		}

		if(text[i] < ' ' || text[i] > '~')
			throw exception("non printing ascii character: " + std::to_string((int)text[i]) + " found in text string");

		// pos vbo
		pos_buffer[(i * 2) + 0] = alignx(parent_->display_width_, parent_->right_ - parent_->left_, xoffset);
		pos_buffer[(i * 2) + 1] = aligny(parent_->display_height_, parent_->bottom_ - parent_->top_, yoffset);

		// texcoord vbo
		const float xnormal = 1.0f / cols;
		const float ynormal = 1.0f / rows;
		const unsigned short xcoord = ((float)((text[i] - ' ') % 16) * xnormal) * USHRT_MAX;
		const unsigned short ycoord = ((float)((text[i] - ' ') / 16) * ynormal) * USHRT_MAX;
		texcoord_buffer[(i * 2) + 0] = xcoord;
		texcoord_buffer[(i * 2) + 1] = 1.0 - ycoord;

		const float bitmap_left = alignx(parent_->display_width_, parent_->right_ - parent_->left_, kern_[text[i] - ' '].bitmap_left);
		xoffset += alignx(parent_->display_width_, parent_->right_ - parent_->left_, kern_[text[i] - ' '].advance) - bitmap_left;
	}

	glBindVertexArray(parent_->vao_);
	glUseProgram(parent_->program_);

	glUniform1f(parent_->uniform_size_, size_);
	glUniform4f(parent_->uniform_color_, r, g, b, a);

	glBindBuffer(GL_ARRAY_BUFFER, parent_->vbo_position_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * textlen * 2, pos_buffer.get(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, parent_->vbo_texcoord_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned short) * 2 * textlen, texcoord_buffer.get(), GL_DYNAMIC_DRAW);

	glBindTexture(GL_TEXTURE_2D, atlas_);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, textlen);
}

void win::font::finalize()
{
	if(atlas_ == 0)
		return;

	glDeleteTextures(1, &atlas_);
}

// font shaders
static const char *vertexshader =
"#version 330 core\n"
"layout (location = 0) in vec2 vert;\n"
"layout (location = 1) in vec2 texcoord_offset;\n"
"layout (location = 2) in vec2 pos;\n"
"layout (location = 3) in vec2 texcoord;\n"
"uniform mat4 projection;\n"
"uniform float size;\n"
"out vec2 ftexcoord;\n"
"void main(){\n"
"ftexcoord = vec2(texcoord.x + texcoord_offset.x, texcoord.y - texcoord_offset.y);\n"
"mat4 translate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x + (size / 2.0), pos.y + (size / 2.0), 0.0, 1.0);\n"
"gl_Position = projection * translate * vec4(vert.x * size, vert.y * size, 0.0, 1.0);\n"
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

// font renderer class
win::font_renderer::font_renderer(int iwidth, int iheight, float left, float right, float bottom, float top)
{
	display_width_ = iwidth;
	display_height_ = iheight;
	left_ = left;
	right_ = right;
	bottom_ = bottom;
	top_ = top;

	// shaders and uniforms
	program_ = load_shaders(vertexshader, fragmentshader);
	glUseProgram(program_);
	uniform_size_ = glGetUniformLocation(program_, "size");
	uniform_color_ = glGetUniformLocation(program_, "color");
	int uniform_projection = glGetUniformLocation(program_, "projection");

	float ortho_matrix[16];
	win::init_ortho(ortho_matrix, left, right, bottom, top);
	glUniformMatrix4fv(uniform_projection, 1, false, ortho_matrix);

	// gen vaos and vbos
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_vertex_);
	glGenBuffers(1, &vbo_position_);
	glGenBuffers(1, &vbo_texcoord_);
	glGenBuffers(1, &ebo_);

	glBindVertexArray(vao_);

	// element indices
	unsigned int indices[] =
	{
		0, 1, 2, 0, 2, 3
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// font vertices
	const float verts[] =
	{
		-0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f, 1.0f / rows,
		0.5f, 0.5f, 1.0f / cols, 1.0f / rows,
		0.5f, -0.5f, 1.0f / cols, 0.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex_);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	// generic attributes
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord_);
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, true, 0, NULL);
}

win::font_renderer::font_renderer(font_renderer &&rhs)
{
	display_width_ = rhs.display_width_;
	display_height_ = rhs.display_height_;
	left_ = rhs.left_;
	right_ = rhs.right_;
	bottom_ = rhs.bottom_;
	top_ = rhs.top_;

	program_ = rhs.program_;
	vao_ = rhs.vao_;
	vbo_vertex_ = rhs.vbo_vertex_;
	vbo_position_ = rhs.vbo_position_;
	vbo_texcoord_ = rhs.vbo_texcoord_;
	uniform_size_ = rhs.uniform_size_;
	uniform_color_ = rhs.uniform_color_;

	rhs.program_ = 0;
	rhs.vao_ = 0;
	rhs.vbo_vertex_ = 0;
	rhs.vbo_position_ = 0;
	rhs.vbo_texcoord_ = 0;
}

win::font_renderer::~font_renderer()
{
	finalize();
}

win::font_renderer &win::font_renderer::operator=(font_renderer &&rhs)
{
	finalize();

	display_width_ = rhs.display_width_;
	display_height_ = rhs.display_height_;
	left_ = rhs.left_;
	right_ = rhs.right_;
	bottom_ = rhs.bottom_;
	top_ = rhs.top_;

	program_ = rhs.program_;
	vao_ = rhs.vao_;
	vbo_vertex_ = rhs.vbo_vertex_;
	vbo_position_ = rhs.vbo_position_;
	vbo_texcoord_ = rhs.vbo_texcoord_;
	uniform_size_ = rhs.uniform_size_;
	uniform_color_ = rhs.uniform_color_;

	rhs.program_ = 0;
	rhs.vao_ = 0;
	rhs.vbo_vertex_ = 0;
	rhs.vbo_position_ = 0;
	rhs.vbo_texcoord_ = 0;

	return *this;
}

win::font win::font_renderer::make_font(resource &rc, float size)
{
	return font(*this, rc, size);
}

void win::font_renderer::finalize()
{
	if(program_ == 0)
		return;

	glDeleteVertexArrays(1, &vao_);
	glDeleteBuffers(1, &vbo_vertex_);
	glDeleteBuffers(1, &vbo_position_);
	glDeleteBuffers(1, &vbo_texcoord_);

	glDeleteProgram(program_);
}
