#include "ListPanel.hpp"

ListPanel::ListPanel(Renderer &renderer, const win::Box<int> &box)
	: renderer(renderer)
	, box(box)
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
			reflow();
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
	// figure out how long since last frame, necessary to figure out
	const auto current_point = std::chrono::high_resolution_clock::now();
	const auto delta_milliseconds = std::chrono::duration<float, std::milli>(current_point - last_point).count();
	last_point = current_point;

	// first, handle item highlighting and marquee effect
	bool found = false;
	for (const auto &item : items)
	{
		if (mouse_is_over(item))
		{
			found = true;
			if (highlighted_item_id != item.id)
			{
				highlighted_item_id = item.id;
				text_xoffset = 0;
			}
			else
			{
				const int text_len = renderer.text_len(item.text.c_str());
				if (((item.x + 6) + text_xoffset) + text_len > (item.x + item.w) - 6)
					text_xoffset -= (delta_milliseconds / 16.66f) * 2;
			}
		}
	}
	if (!found)
		highlighted_item_id = -1;

	// now, draw the panel
	renderer.render(win::Color<unsigned char>(255, 255, 255, 20), box.x, box.y, box.width, box.height);

	if (items.empty())
		renderer.draw_text("< No items >", box.x + (box.width / 2), (box.y + box.height) - 30, true);

	for (const auto &item : items)
	{
		const auto color = item.id == selection_id ? (mouse_is_over(item) ? entry_color_selected : entry_color_selected) : (mouse_is_over(item) ? entry_color_highlighted : entry_color);

		renderer.render(color, item.x, item.y, item.w, item.h);

		renderer.set_drawbox(item.x + 6, item.y, item.w - 12, item.h);
		renderer.draw_text(item.text.c_str(), (item.x + 6) + (mouse_is_over(item) ? text_xoffset : 0), item.y + 8, false);
		renderer.set_drawbox(false);
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

	int y = ((box.y + box.height) - (entry_height)) - 1;
	for (const auto &item : copy)
	{
		const int x = box.x + 1;
		const int w = box.width - 2;
		const int h = entry_height;

		items.emplace_back(item.id, item.text, x, y, w, h);
		y -= entry_height + 1;
	}
}

bool ListPanel::mouse_is_over(const ListEntry &entry) const
{
	return mouse_x >= entry.x && mouse_x <= entry.x + entry.w && mouse_y >= entry.y && mouse_y <= entry.y + entry.h;
}
