#ifndef WIN_FONT_RENDERER_HPP
#define WIN_FONT_RENDERER_HPP

#include <vector>

#include <win/win.hpp>
#include <win/utility.hpp>

#ifdef WIN_USE_OPENGL
#include <GL/gl.h>
#else
#error unimplemented font renderer backend
#endif

namespace win
{

class Font;
class FontRenderer
{
	friend class Font;
public:
	FontRenderer(const IDimensions2D&, const FScreenArea&);
	~FontRenderer();

	WIN_NO_COPY_MOVE(FontRenderer);

	void draw(const Font&, const char *, float, float, const Color&, bool centered = false);

private:
	float line_length(const Font&, const char*, int) const;

#ifdef WIN_USE_OPENGL
	// opengl objects
	GLuint program;
	GLuint vao;
	GLuint vbo_vertex, vbo_position, vbo_texcoord;
	GLuint ebo;
	GLint uniform_size, uniform_color;
#endif

	std::vector<float> pos_buffer;
	std::vector<unsigned short> texcoord_buffer;

	IDimensions2D dims;
	FScreenArea area;
};

}

#endif
