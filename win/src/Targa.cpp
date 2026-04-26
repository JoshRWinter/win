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
    unsigned char id_length;
    raw.read(&id_length, sizeof(id_length));
    if (id_length != 0)
        win::bug("Targa: Can't handle non-zero id length field");

    unsigned char color_map_type;
    raw.read(&color_map_type, sizeof(color_map_type));
    if (color_map_type != 0)
        win::bug("Targa: Color maps are not supported");

    unsigned char image_type;
    raw.read(&image_type, sizeof(image_type));

    if (image_type != 2 && image_type != 3)
        win::bug("Targa: Only uncompressed true-color or grayscale images are supported");

    // width
    raw.seek(12);
    raw.read(&w, sizeof(w));

    // height
    raw.read(&h, sizeof(h));

    // bpp
    raw.read(&bits, sizeof(bits));

    if (bits != 8 && bits != 24 && bits != 32)
        win::bug("Targa: must be 8, 24, or 32 bit color depth");

    // components per pixel
    const int cpp = bits / 8;

    // image descriptor
    unsigned char imdesc;
    raw.read(&imdesc, sizeof(imdesc));

    const int alpha_depth = imdesc & 0b00001111;
    if (alpha_depth != 0 && alpha_depth != 8)
        win::bug("Targa: Only 8 bit alpha components are supported");

    const bool right_to_left = ((imdesc >> 4) & 1) == 1;
    if (right_to_left)
        win::bug("Targa: Right to left ordering is not supported");

    const bool top_origin = ((imdesc >> 5) & 1) == 1;

    const unsigned char interleave_mode = imdesc >> 6;
    if (interleave_mode != 0)
        win::bug("Targa: Interleaving is not supported");

    if (raw.size() - 18 < w * h * cpp)
        win::bug("Targa: Corrupt - tried to read " + std::to_string(w * h * cpp) + " bytes from " + std::to_string(raw.size() - 18) + " bytes");

    raw.seek(18);
    bytes.reset(new unsigned char[w * (h + 1) * cpp]); // "h + 1": account for a bit extra for temp space if we need to flip the image
    raw.read(bytes.get(), w * h * cpp);

    if (top_origin)
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
