#include "Atlasizer.hpp"

void Atlasizer::add(int texture, int w, int h)
{
	items.emplace_back(texture, 0, 0, w, h);
}

const std::vector<AtlasItem> &Atlasizer::get_items() const
{
	return items;
}
