#include "ControlPanel.hpp"

ControlPanel::ControlPanel(Renderer &renderer, int xpos, int ypos, int width, int height)
	: renderer(renderer)
	, xpos(xpos)
	, ypos(ypos)
	, width(width)
	, height(height)
{
	reflow();
}

void ControlPanel::on_import(const std::function<void()> &fn)
{
	fn_import = fn;
}

void ControlPanel::on_export(const std::function<void()> &fn)
{
	fn_export = fn;
}

void ControlPanel::on_add(const std::function<void()> &fn)
{
	fn_add = fn;
}

void ControlPanel::on_remove(const std::function<void()> &fn)
{
	fn_remove = fn;
}

void ControlPanel::on_padding_up(const std::function<void()> &fn)
{
	fn_padding_up = fn;
}

void ControlPanel::on_padding_down(const std::function<void()> &fn)
{
	fn_padding_down = fn;
}

void ControlPanel::mouse_move(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

void ControlPanel::click(bool down)
{
	clicked = down;

	if (!down)
	{
		if (mouse_is_over(load))
			fn_import();
		else if (mouse_is_over(save))
			fn_export();
		else if (mouse_is_over(add))
			fn_add();
		else if(mouse_is_over(remove))
			fn_remove();
		else if (mouse_is_over(padding_up))
			fn_padding_up();
		else if (mouse_is_over(padding_down))
			fn_padding_down();
	}
}

void ControlPanel::set_pad(int p)
{
	padding.text = std::to_string(p);
}

void ControlPanel::draw()
{
	renderer.render(win::Color<unsigned char>(255, 255, 255, 20), xpos, ypos, width, height);

	int text_y_offset = -5;
	draw_button(load, text_y_offset);
	draw_button(save, text_y_offset);
	draw_button(add, text_y_offset);
	draw_button(remove, text_y_offset);
	draw_button(padding_up, text_y_offset - 3);
	draw_button(padding_down, text_y_offset);

	// draw padding box
	draw_border_box(padding, text_y_offset);
}

void ControlPanel::reflow()
{
	constexpr int button_width = 75;
	constexpr int button_height = 30;

	constexpr int small_button_width = 15;
	constexpr int small_button_height = 12;

	int x = xpos + spacing;
	int centerline_y = ypos + (height / 2);

	load = Button("Import", x, centerline_y - (button_height / 2), button_width, button_height);
	x += load.w + spacing;

	save = Button("Export", x, centerline_y - (button_height / 2), button_width, button_height);
	x += save.w + spacing;

	add = Button("Add", x, centerline_y - (button_height / 2), button_width, button_height);
	x += add.w + spacing;

	remove = Button("Remove", x, centerline_y - (button_height / 2), button_width, button_height);
	x += remove.w + (spacing * 2);

	padding = BorderBox("0", win::Color<unsigned char>(255, 255, 255, 40), x, centerline_y - (button_height / 2), button_height, button_height, 1);
	x += padding.w + (spacing / 2);

	padding_up = Button("^", x, centerline_y + 2, small_button_width, small_button_height);
	padding_down = Button("v", x,centerline_y - small_button_height - 2, small_button_width, small_button_height);
}

void ControlPanel::draw_button(const ControlPanel::Button &button, int text_y_offset)
{
	const auto color = mouse_is_over(button) ? (clicked ? button_color_click : button_color_highlight) : button_color;
	renderer.render(color, button.x, button.y, button.w, button.h);
	renderer.draw_text(button.text.c_str(), button.x + (button.w / 2), button.y + (button.h / 2) + text_y_offset, text_color, true);
}

void ControlPanel::draw_border_box(const BorderBox &bb, int text_y_offset)
{
	renderer.render(bb.rgba, bb.x, bb.y, bb.w, bb.weight);
	renderer.render(bb.rgba, bb.x, (bb.y + bb.h) - bb.weight, bb.w, bb.weight);
	renderer.render(bb.rgba, bb.x, (bb.y + bb.weight), bb.weight, bb.h - (bb.weight * 2));
	renderer.render(bb.rgba, bb.x + (bb.w - bb.weight), (bb.y + bb.weight), bb.weight, bb.h - (bb.weight * 2));
	renderer.draw_text(bb.text.c_str(), bb.x + (bb.w / 2), bb.y + (bb.h / 2) + text_y_offset, text_color, true);
}

bool ControlPanel::mouse_is_over(const ControlPanel::Button &button) const
{
	return mouse_x >= button.x && mouse_x <= button.x + button.w && mouse_y >= button.y && mouse_y <= button.y + button.h;
}
