#include <win.h>

win::texture::texture()
{
	object_ = (unsigned)-1;
}

win::texture::texture(texture &&rhs)
{
	object_ = rhs.object_;
	rhs.object_ = (unsigned)-1;
}

win::texture::texture(data raw)
{
	targa(std::move(raw));
}

win::texture::~texture()
{
	finalize();
}

win::texture &win::texture::operator=(texture &&rhs)
{
	finalize();

	object_ = rhs.object_;
	rhs.object_ = -1;

	return *this;
}

win::texture &win::texture::operator=(data raw)
{
	finalize();
	targa(std::move(raw));
	return *this;
}

win::texture::operator unsigned() const
{
	return object_;
}

void win::texture::finalize()
{
	if(object_ == (unsigned)-1)
		return;

	glDeleteTextures(1, &object_);
	object_ = (unsigned)-1;
}

void win::texture::nearest()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void win::texture::linear()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void win::texture::targa(data raw)
{
	const unsigned char *const rawdata = raw.get();

	// compressed?
	unsigned char image_type;
	raw.seek(2);
	if(raw.read(&image_type, sizeof(image_type)) != sizeof(image_type))
		throw exception("Couldn't read image type field");

	const bool compressed = (image_type >> 3) & 1;
	if(compressed)
		throw exception("Compressed TARGAs are not supported");

	// width
	unsigned short width;
	raw.seek(12);
	if(raw.read(&width, sizeof(width)) != sizeof(width))
		throw exception("Couldn't read width field");

	// height
	unsigned short height;
	if(raw.read(&height, sizeof(height)) != sizeof(height))
		throw exception("Couldn't read height field");

	// bpp
	unsigned char bpp;
	raw.seek(16);
	if(raw.read(&bpp, sizeof(bpp)) != sizeof(bpp))
		throw exception("Couldn't read bpp field");

	if(bpp != 32)
		throw exception("Only 32 bit TARGAs are supported");

	// image descriptor
	unsigned char imdesc;
	if(raw.read(&imdesc, sizeof(imdesc)) != sizeof(imdesc))
		throw exception("Couldn't read image description field");

	const bool bottom_origin = !((imdesc >> 5) & 1);

	glGenTextures(1, &object_);
	glBindTexture(GL_TEXTURE_2D, object_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(raw.size() - 18 < width * height * 4)
		throw exception("Corrupt targa: tried to read " + std::to_string(width * height * 4) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

	const int components = bpp == 32 ? 4 : 3;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, components == 4 ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, raw.get() + 18);
}
