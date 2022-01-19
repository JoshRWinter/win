#include <win.h>

namespace win
{

Targa::Targa(AssetRollStream raw)
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

void Targa::load_image_bytes(AssetRollStream &raw)
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

	if(bits != 32)
		win::bug("Only 32 bit TARGAs are supported");

	// image descriptor
	unsigned char imdesc;
	raw.read(&imdesc, sizeof(imdesc));

	const bool bottom_origin = !((imdesc >> 5) & 1);

	if(raw.size() - 18 < w * h * 4)
		win::bug("Corrupt targa: tried to read " + std::to_string(w * h * 4) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

	raw.seek(18);
	bytes.reset(new unsigned char[w * h * 4]);
	raw.read(bytes.get(), w * h * 4);

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

}
