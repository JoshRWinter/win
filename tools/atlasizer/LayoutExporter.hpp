#pragma once

#include <filesystem>
#include <vector>

#include <win/Targa.hpp>
#include <win/FileStream.hpp>

class AtlasItem
{
public:
	AtlasItem(const std::filesystem::path &filepath, int x, int y)
		: filename(filepath)
		, targa(win::Stream(new win::FileStream(filepath)))
		, x(x)
		, y(y)
	{
		bpp = targa.bpp();
		width = targa.width();
		height = targa.height();
	}

public:
	std::filesystem::path filename;
	win::Targa targa;
	int x;
	int y;
	int bpp;
	int width;
	int height;
};

struct AtlasItemDescriptor
{
	std::filesystem::path filename;
	int x;
	int y;
	int width;
	int height;
};

class LayoutExporter
{
public:
	LayoutExporter(const std::filesystem::path &path, int padding);

	void add(const AtlasItemDescriptor &item);
	void save();
	static std::vector<AtlasItemDescriptor> import(const std::filesystem::path &path, int &padding, bool translate_filenames = true);

private:
	std::string serialize_item(const AtlasItemDescriptor&);
	static AtlasItemDescriptor deserialize_item(const std::string&);
	static void get_fields(const std::string&, std::vector<std::string>&);
	static std::vector<std::string> get_fields(const std::string&);

	std::filesystem::path exportfile;
	int padding;
	std::vector<AtlasItemDescriptor> items;
};
