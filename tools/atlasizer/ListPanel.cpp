#include "ListPanel.hpp"

ListPanel::ListPanel(Renderer &renderer, int xpos, int ypos, int width, int height)
	: renderer(renderer)
	, xpos(xpos)
	, ypos(ypos)
	, width(width)
	, height(height)
{}

void ListPanel::add(int id, const std::string &text)
{
	items.emplace_back(id, text, 0, 0, 0, 0);
	reflow();
}

void ListPanel::remove(int id)
{
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		if (it->id == id)
		{
			items.erase(it);
			return;
		}
	}

	win::bug("ListPanel: no item with id " + std::to_string(id));
}

void ListPanel::clear()
{
	items.clear();
}

void ListPanel::set_selection(int id)
{
	selection_id = id;
}

void ListPanel::mouse_move(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

void ListPanel::click(bool down)
{
	for (const auto &item : items)
	{
		if (mouse_is_over(item))
		{
			selection_id = item.id;
			fn_select(selection_id);
		}
	}
}

void ListPanel::draw()
{
	renderer.render(win::Color<unsigned char>(255, 255, 255, 20), xpos, ypos, width, height);

	if (items.empty())
		renderer.draw_text("< No items >", xpos + (width / 2), (ypos + height) - 50, true);

	for (const auto &item : items)
	{
		const auto color = item.id == selection_id ? (mouse_is_over(item) ? entry_color_selected : entry_color_selected) : (mouse_is_over(item) ? entry_color_highlighted : entry_color);

		renderer.render(color, item.x, item.y, item.w, item.h);
		renderer.draw_text(item.text.c_str(), item.x + 6, item.y + 8, false);
	}
}

void ListPanel::on_select(const std::function<void(int id)> &fn)
{
	fn_select = fn;
}

void ListPanel::reflow()
{
	const auto copy = std::move(items);
	items = std::move(std::vector<ListEntry>());

	int y = ((ypos + height) - (entry_height)) - 1;
	for (const auto &item : copy)
	{
		const int x = xpos + 1;
		const int w = width - 2;
		const int h = entry_height;

		items.emplace_back(item.id, item.text, x, y, w, h);
		y -= entry_height + 1;
	}
}

bool ListPanel::mouse_is_over(const ListEntry &entry) const
{
	return mouse_x >= entry.x && mouse_x <= entry.x + entry.w && mouse_y >= entry.y && mouse_y <= entry.y + entry.h;
}
