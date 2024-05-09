#include <filesystem>
#include <string.h>

#include "atlasizer.hpp"
#include "layoutexporter.hpp"

static const char magic[] = { 'A', 'T', 'L', 'A', 'S' };

static bool is_atlas_file(const std::string &path)
{
	std::ifstream test(path, std::ifstream::binary);
	if (!test)
		return false;

	char buf[sizeof(magic)];
	test.read(buf, sizeof(magic));
	if (test.gcount() != sizeof(magic))
		return false;

	for (int i = 0; i < sizeof(magic); ++i)
	{
		if (buf[i] != magic[i])
			return false;
	}

	return true;
}

static void validate_items(const std::vector<AtlasItem> &items)
{
	for (const AtlasItem &item : items)
	{
		if (item.x < 0 || item.y < 0)
			throw std::runtime_error("item " + item.filename + " is out-of-bounds (" + std::to_string(item.x) + ", " + std::to_string(item.y) + ")");
	}
}

static void get_bounds(const std::vector<AtlasItem> &items, int padding, int &width, int &height)
{
	width = 0;
	height = 0;

	for (const AtlasItem &item : items)
	{
		if (item.x + item.width > width)
			width = item.x + item.width;

		if (item.y + item.height > height)
			height = item.y + item.height;
	}

	width += padding;
	height += padding;
}

static void bitblt(const AtlasItem &item, unsigned char *atlas, int width, int height)
{
	int source = 0;
	int dest = (width * 4 * item.y) + (item.x * 4);

	std::unique_ptr<unsigned char[]> converted;
	if (item.targa.bpp() != 32)
	{
		converted = convert_to_bgra8(item.targa);
	}

	const auto *img = converted ? converted.get() : item.targa.data();

	for (int row = 0; row < item.height; ++row)
	{
		if (dest + (item.width * 4) > width * height * 4)
			throw std::runtime_error("internal error: destination overwrite on " + item.filename);
		if (source + (item.width * 4) > item.width * item.height * 4)
			throw std::runtime_error("internal error: source overread on " + item.filename);

		memcpy(atlas + dest, img + source, item.width * 4);

		source += item.width * 4;
		dest += width * 4;
	}
}

void compileatlas(const std::string &layoutfile, const std::string &atlasfile)
{
	if (std::filesystem::exists(atlasfile) && !is_atlas_file(atlasfile))
		throw std::runtime_error("Unwilling to overwrite file " + atlasfile + " which is not an atlas");

	int padding;
	const std::vector<AtlasItemDescriptor> descriptors = LayoutExporter::import(layoutfile, padding);

	std::vector<AtlasItem> items;
	for (const auto &desc : descriptors)
	{
		AtlasItem &added = items.emplace_back(desc.filename, desc.x, desc.y);
		if (added.width != desc.width || added.height != desc.height)
			throw std::runtime_error("item " + desc.filename + " dimensions (" + std::to_string(added.width) + "x" + std::to_string(added.height) + ") do not match original dimensions (" + std::to_string(desc.width) + "x" + std::to_string(desc.height) + ")");
	}

	validate_items(items);

	int width, height;
	get_bounds(items, padding, width, height);

	std::unique_ptr<unsigned char[]> atlas(new unsigned char[width * height * 4]);
	memset(atlas.get(), 0, width * height * 4);

	for (const AtlasItem &item : items)
		bitblt(item, atlas.get(), width, height);

	std::ofstream out(atlasfile, std::ofstream::binary);
	if (!out)
		throw std::runtime_error("couldn't open " + atlasfile + " for writing");

	// write magic
	out.write(magic, sizeof(magic));

	// write item count
	int item_count = items.size();
	out.write((char*)&item_count, sizeof(item_count));

	// write atlas dims
	out.write((char*)&width, sizeof(width));
	out.write((char*)&height, sizeof(height));

	// write locations and dims
	for (const AtlasItem &item : items)
	{
		unsigned short x = item.x;
		unsigned short y = item.y;
		unsigned short w = item.width;
		unsigned short h = item.height;

		out.write((char*)&x, sizeof(x));
		out.write((char*)&y, sizeof(y));
		out.write((char*)&w, sizeof(w));
		out.write((char*)&h, sizeof(h));
	}

	// write atlas
	out.write((char*)atlas.get(), width * height * 4);
}

std::unique_ptr<unsigned char[]> convert_to_bgra8(const win::Targa &targa)
{
	if (targa.bpp() == 32)
		win::bug("no conversion needed");

	std::unique_ptr<unsigned char[]> converted(new unsigned char[targa.width() * targa.height() * 4]);

	const auto original = targa.data();

	if (targa.bpp() == 24)
	{
		for (int i = 0; i < targa.width() * targa.height(); ++i)
		{
			converted[(i * 4) + 0] = original[(i * 3) + 0];
			converted[(i * 4) + 1] = original[(i * 3) + 1];
			converted[(i * 4) + 2] = original[(i * 3) + 2];
			converted[(i * 4) + 3] = 255;
		}
	}
	else if (targa.bpp() == 8)
	{
		for (int i = 0; i < targa.width() * targa.height(); ++i)
		{
			converted[(i * 4) + 0] = original[(i * 1) + 0];
			converted[(i * 4) + 1] = original[(i * 1) + 0];
			converted[(i * 4) + 2] = original[(i * 1) + 0];
			converted[(i * 4) + 3] = 255;
		}
	}
	else
		win::bug("Unsupported color depth " + std::to_string(targa.bpp()));

	return converted;
}
