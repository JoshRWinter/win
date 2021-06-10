#include <climits>
#include <string.h>

#include <win.h>

namespace win
{

Atlas::Atlas(AssetRollStream raw, Mode fm)
{
	int index = 0;

	// check magic
	char magic[6];
	raw.read((unsigned char*)magic, 5);
	index += 5;
	magic[5] = 0;
	if(strcmp(magic, "ATLAS"))
		Atlas::corrupt();

	// how many images
	raw.read(&count, sizeof(count));
	index += sizeof(count);

	std::uint16_t canvas_width = 0;
	std::uint16_t canvas_height = 0;

	raw.read(&canvas_width, sizeof(canvas_width));
	index += sizeof(canvas_width);
	raw.read(&canvas_height, sizeof(canvas_height));
	index += sizeof(canvas_height);

	textures = std::make_unique<AtlasTexture[]>(count);

	for(int i = 0; i < count; ++i)
	{
		std::uint16_t xpos, ypos, width, height;

		raw.read(&xpos, sizeof(xpos));
		index += sizeof(xpos);

		raw.read(&ypos, sizeof(ypos));
		index += sizeof(ypos);

		raw.read(&width, sizeof(width));
		index += sizeof(width);

		raw.read(&height, sizeof(height));
		index += sizeof(height);

		textures[i].coords[0] = ((float)xpos / canvas_width) * USHRT_MAX;
		textures[i].coords[1] = ((float)ypos / canvas_height) * USHRT_MAX;
		textures[i].coords[2] = ((float)(xpos + width) / canvas_width) * USHRT_MAX;
		textures[i].coords[3] = ((float)(ypos + height) / canvas_height) * USHRT_MAX;
	}

	if(raw.size() - index != canvas_width * canvas_height * 4)
		Atlas::corrupt();

	glGenTextures(1, &object);
	glBindTexture(GL_TEXTURE_2D, object);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fm == Mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fm == Mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas_width, canvas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.read_all().get() + index);
}

Atlas::~Atlas()
{
	glDeleteTextures(1, &object);
	object = (unsigned)-1;
}

unsigned Atlas::texture() const
{
	return object;
}

const unsigned short *Atlas::coords(int index) const
{
#ifndef NDEBUG
	if(index >= count || index < 0)
		bug("Bad coords index");
#endif

	return textures[index].coords;
}

void Atlas::corrupt()
{
	win::bug("Corrupt atlas");
}

}
