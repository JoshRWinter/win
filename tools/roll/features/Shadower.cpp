#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <win/FileStream.hpp>
#include <win/Targa.hpp>

#include "Shadower.hpp"

static float decode(unsigned char g)
{
    const float norm = g / 255.0f;
    return norm <= 0.04045f ? norm / 12.92f : std::powf((norm + 0.055f) / 1.055f, 2.4f);
}

static unsigned char encode(float f)
{
    const float encoded = f <= 0.0031308f ? f * 12.92f : ((1.055f * std::powf(f, 1.0f / 2.4f)) - 0.055f);
    return (unsigned char)std::max(0, std::min(255, (int)std::roundf(encoded * 255)));
}

static std::unique_ptr<float[]> decode_img_data(const win::Targa &tga)
{
    std::unique_ptr<unsigned char[]> converted;
    const unsigned char *tgadata = tga.data();

    if (tga.bpp() == 8)
    {
        converted.reset(new unsigned char[tga.width() * tga.height() * 4]);

        for (int s = 0, d = 0; s < tga.width() * tga.height(); s += 1, d += 4)
        {
            converted[d + 0] = tgadata[s + 0];
            converted[d + 1] = tgadata[s + 0];
            converted[d + 2] = tgadata[s + 0];
            converted[d + 3] = 255;
        }

        tgadata = converted.get();
    }
    else if (tga.bpp() == 24)
    {
        converted.reset(new unsigned char[tga.width() * tga.height() * 4]);

        for (int s = 0, d = 0; s < tga.width() * tga.height() * 3; s += 3, d += 4)
        {
            converted[d + 0] = tgadata[s + 0];
            converted[d + 1] = tgadata[s + 1];
            converted[d + 2] = tgadata[s + 2];
            converted[d + 3] = 255;
        }

        tgadata = converted.get();
    }
    else if (tga.bpp() != 32)
        throw std::runtime_error("Shadower: need 32 bpp");

    std::unique_ptr<float[]> data(new float[tga.width() * tga.height() * 4]);
    for (int i = 0; i < tga.width() * tga.height() * 4; i += 4)
    {
        const auto r = decode(tgadata[i + 0]);
        const auto g = decode(tgadata[i + 1]);
        const auto b = decode(tgadata[i + 2]);
        const auto a = tgadata[i + 3] / 255.0f;

        data[i + 0] = r * a;
        data[i + 1] = g * a;
        data[i + 2] = b * a;
        data[i + 3] = a;
    }

    return data;
}

static std::unique_ptr<unsigned char[]> encode_img_data(const float *imgdata, int width, int height)
{
    std::unique_ptr<unsigned char[]> data(new unsigned char[width * height * 4]);
    for (int i = 0; i < width * height * 4; i += 4)
    {
        const auto a = imgdata[i + 3];
        const auto r = imgdata[i + 0] / a;
        const auto g = imgdata[i + 1] / a;
        const auto b = imgdata[i + 2] / a;

        data[i + 0] = encode(r);
        data[i + 1] = encode(g);
        data[i + 2] = encode(b);
        data[i + 3] = std::max(0, std::min(255, (int)std::roundf(a * 255.0f)));
    }

    return data;
}

static void blend(float *dest, int dest_x, int dest_y, int dest_width, int dest_height, const float *src, int src_width, int src_height)
{
    if (dest_x + src_width > dest_width || dest_y + src_height > dest_height)
        throw std::runtime_error("Shadower: bad blend params");

    for (int src_line = 0, dest_line = dest_y; src_line < src_height; ++src_line, ++dest_line)
    {
        const int dest_index = (dest_line * dest_width + dest_x) * 4;
        const int src_index = src_line * src_width * 4;

        for (int i = 0; i < src_width * 4; i += 4)
        {
            const float rs = src[src_index + i + 0];
            const float gs = src[src_index + i + 1];
            const float bs = src[src_index + i + 2];
            const float as = src[src_index + i + 3];

            const float rd = dest[dest_index + i + 0];
            const float gd = dest[dest_index + i + 1];
            const float bd = dest[dest_index + i + 2];
            const float ad = dest[dest_index + i + 3];

            const float r = rs + (rd * (1.0f - as));
            const float g = gs + (gd * (1.0f - as));
            const float b = bs + (bd * (1.0f - as));
            const float a = as + (ad * (1.0f - as));

            dest[dest_index + i + 0] = r;
            dest[dest_index + i + 1] = g;
            dest[dest_index + i + 2] = b;
            dest[dest_index + i + 3] = a;
        }
    }
}

static std::unique_ptr<float[]> embiggen(const float *input, int input_width, int input_height, int output_width, int output_height)
{
    std::unique_ptr<float[]> output(new float[output_width * output_height * 4]);
    memset(output.get(), 0, output_width * output_height * sizeof(float) * 4);

    const int x = (output_width - input_width) / 2;
    const int y = (output_height - input_height) / 2;

    blend(output.get(), x, y, output_width, output_height, input, input_width, input_height);

    return output;
}

void blur(float *img, int width, int height, const std::vector<float> &weights)
{
    struct Pixel
    {
        float r, g, b, a;
    };

    const auto add = [](const Pixel &p1, const Pixel &p2)
    {
        Pixel p;
        p.r = p1.r + p2.r;
        p.g = p1.g + p2.g;
        p.b = p1.b + p2.b;
        p.a = p1.a + p2.a;
        return p;
    };

    const auto weight = [](Pixel p, float w)
    {
        p.r *= w;
        p.g *= w;
        p.b *= w;
        p.a *= w;

        return p;
    };

    const auto set = [width](float *img, int x, int y, const Pixel &p)
    {
        const int index = (y * width + x) * 4;

        img[index + 0] = p.r;
        img[index + 1] = p.g;
        img[index + 2] = p.b;
        img[index + 3] = p.a;
    };

    const auto sample = [width, height](float *img, int x, int y)
    {
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));

        const int index = (y * width + x) * 4;
        Pixel p;
        p.r = img[index + 0];
        p.g = img[index + 1];
        p.b = img[index + 2];
        p.a = img[index + 3];

        return p;
    };

    std::unique_ptr<float[]> tmp(new float[width * height * 4]);
    memset(tmp.get(), 0, width * height * sizeof(float) * 4);

    int x = 0;
    int y = 0;
    for (int i = 0; i < width * height * 4; i += 4)
    {
        if (x >= width)
        {
            x = 0;
            ++y;
        }

        const auto center = sample(img, x, y);
        auto accum = weight(center, weights[0]);

        for (int j = 1; j < weights.size(); ++j)
        {
            const auto left = weight(sample(img, x - j, y), weights[j]);
            const auto right = weight(sample(img, x + j, y), weights[j]);

            accum = add(accum, left);
            accum = add(accum, right);
        }

        set(tmp.get(), x, y, accum);
        ++x;
    }

    x = 0;
    y = 0;
    for (int i = 0; i < width * height * 4; i += 4)
    {
        if (x >= width)
        {
            x = 0;
            ++y;
        }

        const auto center = sample(tmp.get(), x, y);
        auto accum = weight(center, weights[0]);

        for (int j = 1; j < weights.size(); ++j)
        {
            const auto above = weight(sample(tmp.get(), x, y - j), weights[j]);
            const auto below = weight(sample(tmp.get(), x, y + j), weights[j]);

            accum = add(accum, above);
            accum = add(accum, below);
        }

        set(img, x, y, accum);
        ++x;
    }
}

void gray(float *img, int width, int height)
{
    for (int i = 0; i < width * height * 4; i += 4)
    {
        img[i + 0] = 0.0f;
        img[i + 1] = 0.0f;
        img[i + 2] = 0.0f;
    }
}

static void save_tga(const std::filesystem::path &file, const unsigned char *data, int width, int height)
{
    std::ofstream out(file, std::ofstream::binary);
    if (!out)
        throw std::runtime_error("Shadower: couldn't open file " + file.string() + " for reading.");

    unsigned char header[18];
    memset(header, 0, sizeof(header));

    const unsigned char image_type = 2;
    header[2] = image_type;

    const unsigned short w = width;
    const unsigned short h = height;
    const unsigned char bpp = 32;
    memcpy(header + 12, &w, sizeof(w));
    memcpy(header + 14, &h, sizeof(h));
    header[16] = bpp;

    const unsigned char imdesc = 8;
    header[17] = imdesc;

    out.write((char *)header, 18);
    out.write((char *)data, width * height * 4);
}

void shadow(const std::filesystem::path &file, const std::vector<float> &weights)
{
    if (weights.empty())
        return;

    if (!std::filesystem::exists(file))
        throw std::runtime_error("Shadower: no path " + file.string());

    unsigned char header[18];

    {
        std::ifstream in(file, std::ifstream::binary);
        if (!in)
            throw std::runtime_error("Shadower: couldn't open file " + file.string() + " for reading.");

        in.read((char *)header, 18);

        if (in.gcount() != 18)
            throw std::runtime_error("Shadower: couldn't read 18 byte Targa header");

        if (header[0] != 0)
            throw std::runtime_error("Shadower: non-zero Targa image id length not supported");

        if (header[1] != 0)
            throw std::runtime_error("Shadower: Targa color map not supported");

        if (header[2] != 2 && header[2] != 3)
            throw std::runtime_error("Shadower: only uncompressed true-color or grayscale Targas are supported");

        if (header[16] != 8 && header[16] != 24 && header[16] != 32)
            throw std::runtime_error("Shadower: only 8, 24, or 32 bit Targas are supported");

        if ((header[17] & 0b00001111) != 8 && (header[17] & 0b00001111) != 0)
            throw std::runtime_error("Shadower: only 8 bit alpha compenent Targas are supported");

        if (((header[17] >> 4) & 1) == 1)
            throw std::runtime_error("Shadower: right-to-left Targa ordering is not supported");

        if ((header[17] >> 6) != 0)
            throw std::runtime_error("Shadower: Targa interleaving is not supported");
    }

    win::Targa tga(win::Stream(new win::FileStream(file)));

    auto original = decode_img_data(tga);

    const int new_width = tga.width() + (weights.size() - 1) * 2;
    const int new_height = tga.height() + (weights.size() - 1) * 2;

    auto embiggened = embiggen(original.get(), tga.width(), tga.height(), new_width, new_height);

    blur(embiggened.get(), new_width, new_height, weights);
    gray(embiggened.get(), new_width, new_height);
    blend(embiggened.get(), (new_width - tga.width()) / 2, (new_height - tga.height()) / 2, new_width, new_height, original.get(), tga.width(), tga.height());

    auto reencoded = encode_img_data(embiggened.get(), new_width, new_height);
    save_tga(file, reencoded.get(), new_width, new_height);

    std::cout << "Shadower: generated shadow for " << file << " (" << new_width << "x" << new_height << ")" << std::endl;
}
