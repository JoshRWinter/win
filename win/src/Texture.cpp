#include <win.h>

namespace win
{

Texture::Texture()
{
	object = UINT_MAX;
}

Texture::Texture(AssetRollStream raw, TextureMode mode)
{
	targa(raw);

	switch (mode)
	{
	case TextureMode::linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case TextureMode::nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	}
}

Texture::Texture(GLuint texture) : object(texture) { }

Texture::Texture(Texture &&rhs)
{
	move(rhs);
}

Texture::~Texture()
{
	finalize();
}

Texture &Texture::operator=(Texture &&rhs)
{
	finalize();
	move(rhs);

	return *this;
}

void Texture::move(Texture &rhs)
{
	object = rhs.object;
	rhs.object = UINT_MAX;
}

void Texture::finalize()
{
	if (object != UINT_MAX)
		glDeleteTextures(1, &object);
}

unsigned Texture::get() const
{
#ifndef NDEBUG
	if (object == UINT_MAX)
		win::bug("Uninitialized texture");
#endif

	return object;
}

void Texture::targa(AssetRollStream &raw)
{
	// compressed?
	unsigned char image_type;
	raw.seek(2);
	raw.read(&image_type, sizeof(image_type));

	const bool compressed = (image_type >> 3) & 1;
	if(compressed)
		win::bug("Compressed TARGAs are not supported");

	// width
	unsigned short width;
	raw.seek(12);
	raw.read(&width, sizeof(width));

	// height
	unsigned short height;
	raw.read(&height, sizeof(height));

	// bpp
	unsigned char bpp;
	raw.seek(16);
	raw.read(&bpp, sizeof(bpp));

	if(bpp != 32)
		win::bug("Only 32 bit TARGAs are supported");

	// image descriptor
	unsigned char imdesc;
	raw.read(&imdesc, sizeof(imdesc));

	const bool bottom_origin = !((imdesc >> 5) & 1);

	glGenTextures(1, &object);
	glBindTexture(GL_TEXTURE_2D, object);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(raw.size() - 18 < width * height * 4)
		win::bug("Corrupt targa: tried to read " + std::to_string(width * height * 4) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

	const int components = bpp == 32 ? 4 : 3;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, components == 4 ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, raw.read_all().get() + 18);
}

}
