#include "win/gl/GLAtlas.hpp"

#ifdef WIN_USE_OPENGL

namespace win
{

GLAtlas::GLAtlas(Stream stream, Mode mode, GLenum texture_unit)
	: Atlas(std::move(stream))
{
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_2D, gltex.get());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, canvas_width, canvas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, imgdata.get());

	// kill off the bitmap data cause we don't need it any more
	imgdata.reset(NULL);
}

GLuint GLAtlas::texture() const
{
	return gltex.get();
}

}

#endif
