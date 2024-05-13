#include "Atlasizer.hpp"

void Atlasizer::add(int texture, int w, int h)
{
	selection_active = false;
	items.emplace_back(texture, 0, 0, w, h);
}

void Atlasizer::start_drag(int x, int y)
{
	int index = 0;
	for (const auto &item : items)
	{
		if (x >= item.x && x <= item.x + item.w && y >= item.y && y <= item.y + item.h)
		{
			// move it to the back
			AtlasItem copy = item;
			items.erase(items.begin() + index);
			items.push_back(copy);

			selection_active = true;
			grabx = x - copy.x;
			graby = y - copy.y;

			return;
		}

		++index;
	}

	// nothing
	selection_active = false;
}

void Atlasizer::continue_drag(int x, int y)
{
	if (!selection_active)
		return;

	auto &item = *(items.end() - 1);
	item.x = x - grabx;
	item.y = y - graby;
}

const std::vector<AtlasItem> &Atlasizer::get_items() const
{
	return items;
}
