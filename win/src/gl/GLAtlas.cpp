#include "win/gl/GLAtlas.hpp"

#ifdef WIN_USE_OPENGL

namespace win
{

GLAtlas::GLAtlas(Stream stream, Mode mode)
	: Atlas(std::move(stream))
{
	glGenTextures(1, &gltex);
	glBindTexture(GL_TEXTURE_2D, gltex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas_width, canvas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, imgdata.get());

	// kill off the bitmap data cause we don't need it any more
	imgdata.reset(NULL);
}

GLAtlas::~GLAtlas()
{
	glDeleteTextures(1, &gltex);
}

GLuint GLAtlas::texture() const
{
	return gltex;
}

}

#endif
