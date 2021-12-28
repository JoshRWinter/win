#ifndef LAYOUT_EXPORTER_HPP
#define LAYOUT_EXPORTER_HPP

#include <string>
#include <vector>

#include "atlasizer.hpp"

class LayoutExporter
{
public:
	LayoutExporter(const std::string&, int);

	void add(const AtlasItemDescriptor&);
	void save();
	static std::vector<AtlasItemDescriptor> import(const std::string&, int&);

private:
	std::string serialize_item(const AtlasItemDescriptor&);
	static AtlasItemDescriptor deserialize_item(const std::string&);
	static void get_fields(const std::string&, std::vector<std::string>&);
	static std::vector<std::string> get_fields(const std::string&);

	std::string exportfile;
	int padding;
	std::vector<AtlasItemDescriptor> items;
};

#endif
