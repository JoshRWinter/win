#pragma once

#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <array>

#include <win/Utility.hpp>
#include <win/gl/GLFont.hpp>
#include <win/TextRenderer.hpp>
#include <win/gl/GLMappedRingBuffer.hpp>

namespace win
{

class GLTextRenderer : public TextRenderer
{
	WIN_NO_COPY_MOVE(GLTextRenderer);
	friend class Font;

	constexpr static int object_data_length = 2048;

public:
	GLTextRenderer(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area, GLenum texture_unit, bool texture_unit_owned, GLuint shader_storage_block_binding, bool shader_storage_block_binding_owned);

	GLFont create_font(float font_size, Stream data) const;
	void resize(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area);

	void draw(const GLFont &font, const char *text, float xpos, float ypos, bool centered = false);
	void draw(const GLFont &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered = false);

	void flush();

private:
	void send();

	float alignw(float x) const;
	float alignh(float y) const;
	static float align(float f, int pixels, float scale);

	Dimensions<int> screen_pixel_dimensions;
	Area<float> screen_area;

	GLenum texture_unit;
	bool texture_unit_owned;
	GLuint shader_storage_block_binding;
	bool shader_storage_block_binding_owned;

	const void *current_font;
	win::Color<float> current_color;

	GLProgram program;

	GLVertexArray vao;

	GLBuffer vbo_vertex;
	GLBuffer vbo_texcoord;
	GLBuffer vbo_drawids;
	GLBuffer ebo;

	GLBuffer shader_storage_object_data;

	GLint uniform_projection;
	GLint uniform_width;
	GLint uniform_height;
	GLint uniform_color;

	typedef std::array<unsigned char, 16> ObjectBytes;
	GLMappedRingBuffer<ObjectBytes> object_data;
	std::vector<ObjectBytes> object_data_prebuf;
};

}

#endif
