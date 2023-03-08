#include "win/gl/GLAtlas.hpp"

#ifdef WIN_USE_OPENGL

namespace win
{

GLAtlas::GLAtlas(Stream stream, Mode mode)
{

	glGenTextures(1, &gltex);
	glBindTexture(GL_TEXTURE_2D, gltex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode == Mode::linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	Atlas atlas(std::move(stream));

	// copy over the items
	num = atlas.count();
	items.reset(new AtlasItem[num]);
	for (int i = 0; i < num; ++i)
		items[i] = atlas.item(i);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas.width(), atlas.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, atlas.data());
}

GLAtlas::~GLAtlas()
{
	glDeleteTextures(1, &gltex);
}

GLuint GLAtlas::texture() const
{
	return gltex;
}

int GLAtlas::count() const
{
	return num;
}

const AtlasItem &GLAtlas::item(int i) const
{
#ifndef NDEBUG
	if (i < 0 || i >= num)
		win::bug("GLAtlas: item out of bounds");
#endif

	return items[i];
}

}

#endif
