#include <string>
#include <string.h>

#include <win/Win.hpp>
#include <win/Targa.hpp>

namespace win
{

Targa::Targa(Stream raw)
{
	load_image_bytes(raw);
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

void Targa::load_image_bytes(Stream &raw)
{
	// compressed?
	unsigned char image_type;
	raw.seek(2);
	raw.read(&image_type, sizeof(image_type));

	const bool compressed = (image_type >> 3) & 1;
	if(compressed)
		win::bug("Compressed TARGAs are not supported");

	// width
	raw.seek(12);
	raw.read(&w, sizeof(w));

	// height
	raw.read(&h, sizeof(h));

	// bpp
	raw.seek(16);
	raw.read(&bits, sizeof(bits));

	if (bits != 8 && bits != 24 && bits != 32)
		win::bug("TARGAs must be 8, 24, or 32 bit color depth");

	// components per pixel
	const int cpp = bits / 8;

	// image descriptor
	unsigned char imdesc;
	raw.read(&imdesc, sizeof(imdesc));

	const bool bottom_origin = !((imdesc >> 5) & 1);

	if(raw.size() - 18 < w * h * cpp)
		win::bug("Corrupt targa: tried to read " + std::to_string(w * h * cpp) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

	raw.seek(18);
	bytes.reset(new unsigned char[w * h * cpp]);
	raw.read(bytes.get(), w * h * cpp);

	if (!bottom_origin)
	{
		auto newbytes = std::make_unique<unsigned char[]>(w * h * cpp);

		int sourceindex = (w * h * cpp) - (w * cpp);
		for (int i = 0; i < w * h * cpp; i += w * cpp)
		{
			memcpy(newbytes.get() + i, bytes.get() + sourceindex, w * cpp);
			sourceindex -= w * cpp;
		}

		bytes = std::move(newbytes);
	}
}

}
