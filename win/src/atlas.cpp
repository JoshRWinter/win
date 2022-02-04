#include <string.h>

#include <win/win.hpp>
#include <win/atlas.hpp>

#ifdef WIN_USE_OPENGL
#include <GL/gl.h>
#include <GL/glext.h>
#endif

namespace win
{

Atlas::Atlas(Stream raw, Mode fm)
{
	// check magic
	char magic[6];
	raw.read((unsigned char*)magic, 5);
	magic[5] = 0;
	if(strcmp(magic, "ATLAS"))
		Atlas::corrupt();

	// how many images
	raw.read(&count, sizeof(count));

	int canvas_width = 0;
	int canvas_height = 0;
	raw.read(&canvas_width, sizeof(canvas_width));
	raw.read(&canvas_height, sizeof(canvas_height));

	textures = std::make_unique<AtlasItem[]>(count);

	for(int i = 0; i < count; ++i)
	{
		std::uint16_t xpos, ypos, width, height;

		raw.read(&xpos, sizeof(xpos));

		raw.read(&ypos, sizeof(ypos));

		raw.read(&width, sizeof(width));

		raw.read(&height, sizeof(height));

		textures[i].x1 = (float)xpos / canvas_width;
		textures[i].y1 = (float)ypos / canvas_height;
		textures[i].x2 = (float)(xpos + width) / canvas_width;
		textures[i].y2 = (float)(ypos + height) / canvas_height;
	}

	if(raw.size() - raw.tell() != canvas_width * canvas_height * 4)
		Atlas::corrupt();

#ifdef WIN_USE_OPENGL
	glGenTextures(1, &object);
	glBindTexture(GL_TEXTURE_2D, object);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fm == Mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fm == Mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::unique_ptr<unsigned char[]> pixels(new unsigned char[canvas_width * canvas_height * 4]);
	raw.read(pixels.get(), canvas_width * canvas_height * 4);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas_width, canvas_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels.get());
#endif
}

Atlas::~Atlas()
{
	glDeleteTextures(1, &object);
}

#ifdef WIN_USE_OPENGL
GLuint Atlas::texture() const
{
	return object;
}
#endif

const AtlasItem Atlas::item(int index) const
{
#ifndef NDEBUG
	if(index >= count || index < 0)
		win::bug("Bad coords index");
#endif

	return textures[index];
}

void Atlas::corrupt()
{
	win::bug("Corrupt atlas");
}

}
