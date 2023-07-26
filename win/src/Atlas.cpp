#include <cstdint>
#include <cstring>

#include <win/Win.hpp>
#include <win/Atlas.hpp>

#ifdef WIN_USE_OPENGL
#include <GL/gl.h>
#include <GL/glext.h>
#endif

namespace win
{

Atlas::Atlas(Stream raw)
{
	// check magic
	char magic[6];
	raw.read((unsigned char*)magic, 5);
	magic[5] = 0;
	if(strcmp(magic, "ATLAS"))
		Atlas::corrupt();

	// how many images
	raw.read(&num, sizeof(num));

	raw.read(&canvas_width, sizeof(canvas_width));
	raw.read(&canvas_height, sizeof(canvas_height));

	textures = std::make_unique<AtlasItem[]>(num);

	for(int i = 0; i < num; ++i)
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

	imgdata.reset(new unsigned char[canvas_width * canvas_height * 4]);
	raw.read(imgdata.get(), canvas_width * canvas_height * 4);
}

int Atlas::count() const
{
	return num;
}

const AtlasItem &Atlas::item(int index) const
{
#ifndef NDEBUG
	if(index >= num || index < 0)
		win::bug("Atlas: item out of bounds");
#endif

	return textures[index];
}

void Atlas::corrupt()
{
	win::bug("Atlas: corrupt");
}

}
