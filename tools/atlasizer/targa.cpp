#include <string.h>

#include <win.h>

#include "targa.hpp"


static void corrupt()
{
	throw new std::runtime_error("Targa appears to be corrupt");
}

static unsigned long long filesize(const std::string &fname)
{
	return 0;
}

Targa::Targa(const std::string &filename)
	: filename(filename)
{
	std::ifstream file(filename);
	if (!file)
		throw std::runtime_error(std::string(filename) + "doesn't exist");
	load_image_bytes(file);
}

int Targa::width() const
{
	return w;
}

int Targa::height() const
{
	return h;
}

int Targa::bpp() const
{
	return bits;
}

const unsigned char *Targa::data() const
{
	return bytes.get();
}

unsigned char *Targa::release()
{
	return bytes.release();
}

void Targa::load_image_bytes(std::ifstream &stream)
{
	// compressed?
	unsigned char image_type;
	stream.seekg(2);
	stream.read((char*)&image_type, sizeof(image_type));
	if (stream.gcount() != sizeof(image_type))
		corrupt();

	const bool compressed = (image_type >> 3) & 1;
	if(compressed)
		throw std::runtime_error("Compressed TARGAs are not supported");

	// width
	stream.seekg(12);
	stream.read((char*)&w, sizeof(w));
	if (stream.gcount() != sizeof(w))
		corrupt();

	// height
	stream.read((char*)&h, sizeof(h));
	if (stream.gcount() != sizeof(h))
		corrupt();

	// bpp
	stream.seekg(16);
	stream.read((char*)&bits, sizeof(bits));
	if (stream.gcount() != sizeof(bits))
		corrupt();

	// image descriptor
	unsigned char imdesc;
	stream.read((char*)&imdesc, sizeof(imdesc));
	if (stream.gcount() != sizeof(imdesc))
		corrupt();

	const bool bottom_origin = !((imdesc >> 5) & 1);

	if(filesize(filename) - 18 < w * h * 4)
		corrupt();//("Corrupt targa: tried to read " + std::to_string(w * h * 4) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

	stream.seekg(18);
	bytes.reset(new unsigned char[w * h * 4]);
	stream.read((char*)bytes.get(), w * h * 4);

	if(bits == 24)
	{
		auto newbytes = std::make_unique<unsigned char[]>(w * h * 4);

		int sourceindex = 0;
		for (int i = 0; i < w * h * 4; i += 4)
		{
			newbytes[i + 0] = bytes[sourceindex + 0];
			newbytes[i + 1] = bytes[sourceindex + 1];
			newbytes[i + 2] = bytes[sourceindex + 2];
			newbytes[i + 3] = 255;

			sourceindex += 3;
		}

		bytes = std::move(newbytes);
	}
	else if (bits != 32)
		win::bug("TARGAs must be 24 or 32 bits");

	if (!bottom_origin)
	{
		auto newbytes = std::make_unique<unsigned char[]>(w * h * 4);

		int sourceindex = (w * h * 4) - (w * 4);
		for (int i = 0; i < w * h * 4; i += w * 4)
		{
			memcpy(newbytes.get() + i, bytes.get() + sourceindex, w * 4);
			sourceindex -= w * 4;
		}

		bytes = std::move(newbytes);
	}
}
