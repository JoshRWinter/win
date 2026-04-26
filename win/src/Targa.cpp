#include <string.h>
#include <string>

#include <win/Targa.hpp>
#include <win/Win.hpp>

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
    if (compressed)
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

    if (raw.size() - 18 < w * h * cpp)
        win::bug("Corrupt targa: tried to read " + std::to_string(w * h * cpp) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

    raw.seek(18);
    bytes.reset(new unsigned char[w * (h + 1) * cpp]); // "h + 1": account for a bit extra for temp space if we need to flip the image
    raw.read(bytes.get(), w * h * cpp);

    if (!bottom_origin)
    {
        unsigned char *temp = bytes.get() + (w * h * cpp); // use the extra space at the end

        for (int i = 0; i < h / 2; ++i)
        {
            const auto line1 = i;
            const auto line2 = (h - 1) - i;

            const int index1 = line1 * w * cpp;
            const int index2 = line2 * w * cpp;

            memcpy(temp, bytes.get() + index2, w * cpp);
            memcpy(bytes.get() + index2, bytes.get() + index1, w * cpp);
            memcpy(bytes.get() + index1, temp, w * cpp);
        }
    }
}

}
