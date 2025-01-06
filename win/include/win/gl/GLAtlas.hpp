#pragma once

#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <GL/gl.h>
#include <GL/glext.h>

#include <win/Atlas.hpp>
#include <win/gl/GL.hpp>

namespace win
{

class GLAtlas : public Atlas
{
	WIN_NO_COPY(GLAtlas);

public:
	enum class Mode { linear, nearest };

	GLAtlas(Stream, Mode, GLenum texture_unit);
	GLAtlas(GLAtlas&&) = default;

	GLuint texture() const;

private:
	GLTexture gltex;

	std::unique_ptr<AtlasItem[]> items;
};

}

#endif
