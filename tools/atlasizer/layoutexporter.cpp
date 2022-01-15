#include <filesystem>
#include <stdio.h>

#include "layoutexporter.hpp"

LayoutExporter::LayoutExporter(const std::string &exportfile, int padding)
	: exportfile(exportfile)
	, padding(padding)
{
}

void LayoutExporter::add(const AtlasItemDescriptor &item)
{
	items.push_back(item);
}

void LayoutExporter::save()
{
	std::ofstream out(exportfile);
	if (!out)
		throw std::runtime_error("Couldn't open " + exportfile + " for writing");

	out << "$padding " + std::to_string(padding) << std::endl;

	for (const AtlasItemDescriptor &item : items)
	{
		out << serialize_item(item) << std::endl;
	}
}

std::vector<AtlasItemDescriptor> LayoutExporter::import(const std::string &file, int &padding)
{
	std::vector<AtlasItemDescriptor> items;

	std::ifstream in(file);
	if (!in)
		throw std::runtime_error("Couldn't open " + file + " for reading");

	std::string str_padding_line;
	std::getline(in, str_padding_line);
	if (str_padding_line.find("$padding ") != 0)
		throw std::runtime_error("Couldn't parse padding line");

	const std::string str_padding_int = str_padding_line.substr(9);
	if (str_padding_int.length() < 1)
		throw std::runtime_error("Couldn't parse padding int");

	if (sscanf(str_padding_int.c_str(), "%d", &padding) != 1)
		throw std::runtime_error("Couldn't parse padding int");

	while (!in.eof())
	{
		std::string line;
		std::getline(in, line);

		if (line.size() < 1)
			continue;

		AtlasItemDescriptor aid = deserialize_item(line);
		aid.filename = std::filesystem::relative(std::filesystem::path(file).parent_path() / std::filesystem::path(aid.filename));
		items.push_back(aid);
	}

	return items;
}

std::string LayoutExporter::serialize_item(const AtlasItemDescriptor &item)
{
	std::filesystem::path path = std::filesystem::path(exportfile).parent_path();
	std::filesystem::path newname = std::filesystem::relative(item.filename, path);

	return newname.string() + ":" + std::to_string(item.x) + ":" + std::to_string(item.y) + ":" + std::to_string(item.width) + ":" + std::to_string(item.height);
}

AtlasItemDescriptor LayoutExporter::deserialize_item(const std::string &line)
{
	std::vector<std::string> fields = get_fields(line);

	if (fields.size() != 5)
	    throw std::runtime_error("Couldn't parse atlas line");

	int x, y, w, h;

	if (sscanf(fields.at(1).c_str(), "%d", &x) != 1)
		throw std::runtime_error("Couldn't parse atlas x");
	if (sscanf(fields.at(2).c_str(), "%d", &y) != 1)
		throw std::runtime_error("Couldn't parse atlas y");
	if (sscanf(fields.at(3).c_str(), "%d", &w) != 1)
		throw std::runtime_error("Couldn't parse atlas width");
	if (sscanf(fields.at(4).c_str(), "%d", &h) != 1)
		throw std::runtime_error("Couldn't parse atlas height");

	AtlasItemDescriptor item;
	item.filename = fields.at(0);
	item.x = x;
	item.y = y;
	item.width = w;
	item.height = h;

	return item;
}

void LayoutExporter::get_fields(const std::string &line, std::vector<std::string> &fields)
{
    const auto pos = line.find(":");
	if (pos == std::string::npos)
	{
		if (line.length() > 0)
			fields.push_back(line);
		return;
	}

	fields.push_back(line.substr(0, pos));
	get_fields(line.substr(pos + 1), fields);
}

std::vector<std::string> LayoutExporter::get_fields(const std::string &line)
{
	std::vector<std::string> fields;
	get_fields(line, fields);
	return fields;
}
