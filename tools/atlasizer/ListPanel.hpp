#pragma once

#include <vector>
#include <functional>
#include <chrono>

#include <win/Win.hpp>

#include "Renderer.hpp"

class ListPanel
{
	WIN_NO_COPY_MOVE(ListPanel);

	struct ListEntry
	{
		ListEntry(int id, const std::string &text, int x, int y, int w, int h) : id(id), text(text), x(x), y(y), w(w), h(h) {}

		int id;
		std::string text;
		int x, y, w, h;
	};

	constexpr static int entry_height = 30;

	inline static win::Color<unsigned char> entry_color = win::Color<unsigned char>(90, 90, 90, 200);
	inline static win::Color<unsigned char> entry_color_highlighted = win::Color<unsigned char>(130, 130, 130, 200);
	inline static win::Color<unsigned char> entry_color_selected = win::Color<unsigned char>(70, 70, 70, 200);

public:
	ListPanel(Renderer &renderer, const win::Box<int> &box);

	void add(int id, const std::string &text);
	void remove(int id);
	void clear();
	void set_selection(int id);

	void mouse_move(int x, int y);
	void click(bool down);
	void draw();

	void on_select(const std::function<void(int id)> &fn);

private:
	void reflow();
	bool mouse_is_over(const ListEntry &entry) const;
	static win::Box<int> drawbox_intersection(const win::Box<int> &base, const win::Box<int> &box);

	Renderer &renderer;
	win::Box<int> box;

	std::chrono::time_point<std::chrono::high_resolution_clock> last_point = std::chrono::high_resolution_clock::now();

	int highlighted_item_id = -1;
	float text_xoffset = 0.0f;
	float scroll_yoffset = 0.0f;

	int mouse_x = 0, mouse_y = 0;
	int selection_id = -1;

	std::vector<ListEntry> items;

	std::function<void(int id)> fn_select;
};
