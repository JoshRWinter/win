#include "Atlasizer.hpp"

int Atlasizer::next_atlasitem_id = 0;

void Atlasizer::add(int texture, const std::filesystem::path &texturepath, int x, int y, int w, int h)
{
	selection_active = false;

	const int xpos = std::max(0, x);
	const int ypos = std::max(0, y);

	items.emplace_back(next_atlasitem_id++, texture, texturepath, xpos, ypos, w, h);

	check_validity();
}

void Atlasizer::remove(int id)
{
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		if (it->id == id)
		{
			items.erase(it);
			return;
		}
	}

	win::bug("No item in atlas with id " + std::to_string(id));
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
				AtlasizerItem copy = item;
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

void Atlasizer::continue_drag(int x, int y, bool snap)
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

	if (snap)
	{
		// possibly delete drag barriers
		if (left_barrier.has_value() && item.x > left_barrier.value())
			left_barrier.reset();
		if (right_barrier.has_value() && item.x < right_barrier.value())
			right_barrier.reset();
		if (bottom_barrier.has_value() && item.y > bottom_barrier.value())
			bottom_barrier.reset();
		if (top_barrier.has_value() && item.y < top_barrier.value())
			top_barrier.reset();

		// possibly force compliance with drag barriers
		if (left_barrier.has_value() && item.x < left_barrier.value())
			item.x = left_barrier.value();
		if (right_barrier.has_value() && item.x > right_barrier.value())
			item.x = right_barrier.value();
		if (bottom_barrier.has_value() && item.y < bottom_barrier.value())
			item.y = bottom_barrier.value();
		if (top_barrier.has_value() && item.y > top_barrier.value())
			item.y = top_barrier.value();

		// possibly create drag barriers
		for (const auto &item2 : items)
		{
			if (&item2 == &item)
				continue;

			if (collide(item, item2))
			{
				const auto side = collision_side(item, item2);

				switch (side)
				{
					case CollisionSide::left:
						item.x = item2.x + item2.w + padding;
						right_barrier.reset();
						left_barrier.emplace(item.x);
						break;
					case CollisionSide::right:
						item.x = item2.x - (item.w + padding);
						left_barrier.reset();
						right_barrier.emplace(item.x);
						break;
					case CollisionSide::bottom:
						item.y = item2.y + item2.h + padding;
						top_barrier.reset();
						bottom_barrier.emplace(item.y);
						break;
					case CollisionSide::top:
						item.y = item2.y - (item.h + padding);
						bottom_barrier.reset();
						top_barrier.emplace(item.y);
						break;
				}
			}
		}
	}
	else
	{
		left_barrier.reset();
		right_barrier.reset();
		bottom_barrier.reset();
		top_barrier.reset();
	}

	check_validity();
}

void Atlasizer::set_padding(int pad)
{
	padding = pad;
	check_validity();
}

int Atlasizer::get_padding() const
{
	return padding;
}

const std::vector<AtlasizerItem> &Atlasizer::get_items() const
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

			if (collide(item, item2))
			{
				item.valid = false;
				item2.valid = false;
			}
		}
	}
}

bool Atlasizer::collide(const AtlasizerItem &a, const AtlasizerItem &b) const
{
	return
		a.x + a.w + padding > b.x &&
		a.x < b.x + b.w + padding &&
		a.y + a.h + padding > b.y &&
		a.y < b.y + b.h + padding;
}

Atlasizer::CollisionSide Atlasizer::collision_side(const AtlasizerItem &a, const AtlasizerItem &b) const
{
	if (!collide(a, b))
		win::bug("no collision");

	const int left = std::abs(a.x - (b.x + b.w + padding));
	const int right = std::abs((a.x + a.w + padding) - b.x);
	const int bottom = std::abs(a.y - (b.y + b.h + padding));
	const int top = std::abs((a.y + a.h + padding) - b.y);

	const int min = std::min(left, std::min(right, std::min(bottom, top)));

	if (min == left)
		return CollisionSide::left;
	else if (min == right)
		return CollisionSide::right;
	else if (min == bottom)
		return CollisionSide::bottom;
	else if (min == top)
		return CollisionSide::top;
	else
		win::bug("no side");
}
