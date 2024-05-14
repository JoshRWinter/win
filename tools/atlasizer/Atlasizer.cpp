#include "Atlasizer.hpp"

void Atlasizer::add(int texture, int w, int h)
{
	selection_active = false;
	items.emplace_back(texture, 0, 0, w, h);

	check_validity();
}

void Atlasizer::start_drag(int x, int y)
{
	int index = -1;
	for (int i = (int)items.size() - 1; i >= 0; --i)
	{
		const auto &item = items.at(i);

		if (x > item.x && x < item.x + item.w && y > item.y && y < item.y + item.h)
		{
			// move it to the back
			if (i != items.size() - 1)
			{
				AtlasItem copy = item;
				items.erase(items.begin() + i);
				items.push_back(copy);
			}

			index = items.size() - 1;

			break;
		}
	}

	if (index != -1)
	{
		auto &item = items.at(index);

		selection_active = true;
		grabx = x - item.x;
		graby = y - item.y;
	}
	else
	{
		// nothing
		selection_active = false;
	}
}

void Atlasizer::continue_drag(int x, int y)
{
	if (!selection_active)
		return;

	auto &item = *(items.end() - 1);
	item.x = x - grabx;
	item.y = y - graby;

	if (item.x < 0)
		item.x = 0;
	if (item.y < 0)
		item.y = 0;

	check_validity();
}

const std::vector<AtlasItem> &Atlasizer::get_items() const
{
	return items;
}

void Atlasizer::check_validity()
{
	for (auto &item : items) item.valid = true;
	for (auto &item : items)
	{
		for (auto &item2 : items)
		{
			if (&item == &item2)
				continue;

			if (item.x + item.w > item2.x && item.x < item2.x + item2.w && item.y + item.h > item2.y && item.y < item2.y + item2.h)
			{
				item.valid = false;
				item2.valid = false;
			}
		}
	}
}
