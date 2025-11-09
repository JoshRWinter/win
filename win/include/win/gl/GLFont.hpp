#pragma once

#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <win/gl/GL.hpp>
#include <win/Font.hpp>

namespace win
{

class GLFont : public Font
{
	WIN_NO_COPY(GLFont);

	friend class GLTextRenderer;

	GLFont(const Dimensions<int> &screen_dimensions, const Area<float> &screen_area, float font_size, Stream font_file);

public:
	GLFont(GLFont&&) = default;

	GLFont &operator=(GLFont&&) = default;

	GLuint texture() const { return tex.get(); }

private:
	GLTexture tex;
};

}
#endif
