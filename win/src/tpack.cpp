#include <win.h>

win::tpack::tpack()
{
	count_ = 0;
}

win::tpack::tpack(const data_list &list, mode filter_mode)
{
	count_ = list.count();
	textures_ = std::make_unique<unsigned[]>(count_);

	glGenTextures(count_, textures_.get());

	for(int i = 0; i < count_; ++i)
	{
		try
		{
			tpack::targa(list.get(i), textures_[i], filter_mode);
		}
		catch(const exception &e)
		{
			throw exception("Corrupt tga file \"" + list.name(i) + "\": " + e.what());
		}
	}
}

win::tpack::tpack(tpack &&rhs)
{
	move(rhs);
}

win::tpack::~tpack()
{
	finalize();
}

win::tpack &win::tpack::operator=(tpack &&rhs)
{
	finalize();
	move(rhs);
	return *this;
}

unsigned win::tpack::operator[](int index) const
{
#ifndef NDEBUG
	if(index >= count_ || index < 0)
		bug("tpack index " + std::to_string(index) + " not in [0, " + std::to_string(count_) + ")");
#endif

	return textures_[index];
}

void win::tpack::filter(int index, mode filter_mode)
{
#ifndef NDEBUG
	if(index >= count_ || index < 0)
		bug("tpack index " + std::to_string(index) + " not in [0, " + std::to_string(count_) + ")");
#endif

	glBindTexture(GL_TEXTURE_2D, textures_[index]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
}

void win::tpack::targa(data raw, const unsigned object, mode filter_mode)
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

	if(bpp != 32 && bpp != 24)
		throw exception("Only 24bit and 32bit TARGAs are supported");

	// image descriptor
	unsigned char imdesc;
	if(raw.read(&imdesc, sizeof(imdesc)) != sizeof(imdesc))
		throw exception("Couldn't read image description field");

	const bool bottom_origin = !((imdesc >> 5) & 1);

	glBindTexture(GL_TEXTURE_2D, object);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode == mode::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	const int components = bpp == 32 ? 4 : 3;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, components == 4 ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, raw.get() + 18);
}

void win::tpack::move(tpack &rhs)
{
	count_ = rhs.count_;
	rhs.count_ = 0;

	textures_ = std::move(rhs.textures_);
}

void win::tpack::finalize()
{
	if(count_ == 0)
		return;

	glDeleteTextures(count_, textures_.get());
	count_ = 0;
}
