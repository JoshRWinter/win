#pragma once

#include <functional>

#include <win/Win.hpp>

#include "Renderer.hpp"

class ControlPanel
{
	WIN_NO_COPY_MOVE(ControlPanel);

	static constexpr int spacing = 10;

	inline static const win::Color<unsigned char> button_color = win::Color<unsigned char>(255, 255, 255, 50);
	inline static const win::Color<unsigned char> button_color_highlight = win::Color<unsigned char>(255, 255, 255, 80);
	inline static const win::Color<unsigned char> button_color_click = win::Color<unsigned char>(255, 255, 255, 40);

	inline static const win::Color<unsigned char> text_color = win::Color<unsigned char>(0, 0, 0, 255);

	struct Button
	{
		Button() = default;
		Button(const std::string &text, int x, int y, int w, int h) : text(text), x(x), y(y), w(w), h(h) {}

		std::string text;
		int x = 0, y = 0, w = 0, h = 0;
	};

	struct BorderBox
	{
		BorderBox() = default;
		BorderBox(const std::string &text, win::Color<unsigned char> rgba, int x, int y, int w, int h, int weight) : text(text), rgba(rgba), x(x), y(y), w(w), h(h), weight(weight) {}

		std::string text;
		win::Color<unsigned char> rgba;
		int x = 0, y = 0, w = 0, h = 0, weight = 0;
	};

public:
	explicit ControlPanel(Renderer &renderer, int xpos, int ypos, int width, int height);

	void on_import(const std::function<void()> &fn);
	void on_export(const std::function<void()> &fn);
	void on_add(const std::function<void()> &fn);
	void on_remove(const std::function<void()> &fn);
	void on_padding_up(const std::function<void()> &fn);
	void on_padding_down(const std::function<void()> &fn);

	void mouse_move(int x, int y);
	void click(bool down);
	void set_pad(int p);
	void draw();

private:
	void reflow();
	void draw_button(const Button &button, int text_y_offset);
	void draw_border_box(const BorderBox &bb, int text_y_offset);
	bool mouse_is_over(const Button &button) const;

	Renderer &renderer;
	int xpos, ypos, width, height;

	int mouse_x = 0, mouse_y = 0;
	int clicked = false;

	Button load, save, add, remove, padding_up, padding_down;
	BorderBox padding;

	std::function<void()> fn_import = [](){};
	std::function<void()> fn_export = [](){};
	std::function<void()> fn_add = [](){};
	std::function<void()> fn_remove = [](){};
	std::function<void()> fn_padding_up = [](){};
	std::function<void()> fn_padding_down = [](){};
};
