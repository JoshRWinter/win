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

	const float frequency_multiplier = (delta_milliseconds / 16.666f);

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
					text_xoffset -= frequency_multiplier * 2;
			}
		}
	}
	if (!found)
		highlighted_item_id = -1;

	// handle scrolling
	if (items.empty())
		scroll_yoffset = 0.0f;
	else if (mouse_x >= box.x && mouse_x < box.x + box.width && mouse_y >= box.y && mouse_y < box.y + box.height)
	{
		const int scroll_zone = 40;
		const float step = frequency_multiplier * 5.0f;

		if (mouse_y > (box.y + box.height) - scroll_zone)
			scroll_yoffset -= step;
		else if (mouse_y < box.y + scroll_zone)
			scroll_yoffset += step;

		const auto &first = *items.begin();
		const auto &last = *(items.end() - 1);

		const int list_top = first.y + first.h;
		const int list_height = list_top - last.y;
		const int scroll_yoffset_max = (box.y + list_height) - (box.y + box.height);

		scroll_yoffset = std::min(scroll_yoffset, (float)scroll_yoffset_max);
		scroll_yoffset = std::max(scroll_yoffset, 0.0f);
	}

	const win::Box<int> base_drawbox(box.x, box.y, box.width, box.height);

	// now, draw the panel
	renderer.render(win::Color<unsigned char>(255, 255, 255, 20), box.x, box.y, box.width, box.height);

	if (items.empty())
		renderer.draw_text("< No items >", box.x + (box.width / 2), (box.y + box.height) - 30, true);

	for (const auto &item : items)
	{
		const auto color = item.id == selection_id ? (mouse_is_over(item) ? entry_color_selected : entry_color_selected) : (mouse_is_over(item) ? entry_color_highlighted : entry_color);

		renderer.set_drawbox(base_drawbox);
		renderer.render(color, item.x, item.y + scroll_yoffset, item.w, item.h);

		const win::Box<int> text_drawbox(item.x + 6, item.y + scroll_yoffset, item.w - 12, item.h);
		renderer.set_drawbox(drawbox_intersection(base_drawbox, text_drawbox));
		renderer.draw_text(item.text.c_str(), (item.x + 6) + (mouse_is_over(item) ? text_xoffset : 0), (item.y + 8) + scroll_yoffset, false);
	}

	renderer.disable_drawbox();
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
	return mouse_x >= box.x && mouse_y >= box.y && mouse_x < box.x + box.width && mouse_y < box.y + box.height &&
	mouse_x >= entry.x && mouse_x < entry.x + entry.w && mouse_y >= entry.y + scroll_yoffset && mouse_y < (entry.y + entry.h) + scroll_yoffset;
}

win::Box<int> ListPanel::drawbox_intersection(const win::Box<int> &base, const win::Box<int> &box)
{
	const int left = std::max(base.x, box.x);
	const int bottom = std::max(base.y, box.y);
	const int right = std::min(base.x + base.width, box.x + box.width);
	const int top = std::min(base.y + base.height, box.y + box.height);

	const int width = std::max(0, right - left);
	const int height = std::max(0, top - bottom);

	return win::Box<int>(left, bottom, width, height);
}
