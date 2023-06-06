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
	WIN_NO_COPY(GLTextRenderer);
	friend class Font;

	constexpr static int object_data_multiplier = 3;
	constexpr static int object_data_length = 100;
	constexpr static int uniform_object_data_length = object_data_length * object_data_multiplier;

public:
	GLTextRenderer(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area);

	void draw(const GLFont &font, const char *text, float xpos, float ypos, bool centered = false);
	void draw(const GLFont &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered = false);

	void flush();

private:
	GLProgram program;

	GLVertexArray vao;

	GLBuffer vbo_vertex;
	GLBuffer vbo_texcoord;
	GLBuffer vbo_drawids;
	GLBuffer ebo;

	GLBuffer uniform_object_data;

	GLint uniform_projection;
	GLint uniform_width;
	GLint uniform_height;
	GLint uniform_color;
	GLint uniform_blink;

	typedef std::array<unsigned char, 16> ObjectBytes;
	GLMappedRingBuffer<ObjectBytes> object_data;
};

}

#endif
