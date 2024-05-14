#include <win/Display.hpp>
#include <win/AssetRoll.hpp>
#include <win/FileReadStream.hpp>

#include "Renderer.hpp"
#include "Atlasizer.hpp"

void gui2()
{
	win::DisplayOptions options;
	options.width = 800;
	options.height = 600;
	options.gl_major = 4;
	options.gl_minor = 4;
	options.caption = "Atlasizer";

	win::Display display(options);
	win::load_gl_functions();

	win::AssetRoll roll("atlasizer.roll");
	Renderer renderer(roll, display.width(), display.height());
	Atlasizer atlasizer;

	// interface state
	enum DragMode { none, pan, drag } drag_mode = DragMode::none;
	int center_x = -150, center_y = -150;
	float zoom = 1.0f;
	int mouse_x = 0, mouse_y = 0, mouse_world_x = 0, mouse_world_y = 0;

	renderer.set_view(center_x, center_y, zoom);

	display.register_button_handler([&](const win::Button button, const bool press)
	{
		switch (button)
		{
			case win::Button::mouse_right:
				if (press)
				{
					if (drag_mode == DragMode::none)
						drag_mode = DragMode::pan;
				}
				else
				{
					drag_mode = DragMode::none;
				}
				break;
			case win::Button::mouse_left:
				if (press)
				{
					if (drag_mode == DragMode::none)
					{
						drag_mode = DragMode::drag;
						atlasizer.start_drag(mouse_world_x, mouse_world_y);
					}
				}
				else
				{
					drag_mode = DragMode::none;
				}
				break;
			case win::Button::a:
				if (press)
				{
					win::Targa tga(win::Stream(new win::FileReadStream("/home/josh/programming/darktimes/asset/texture/chair1.tga")));
					atlasizer.add(renderer.add_texture(tga), tga.width(), tga.height());
					break;
				}
			case win::Button::num_plus:
				if (drag_mode == DragMode::none && press)
				{
					zoom += 0.1f;
					renderer.set_view(center_x, center_y, zoom);
				}
				break;
			case win::Button::num_minus:
				if (drag_mode == DragMode::none && press)
				{
					zoom -= 0.1f;
					renderer.set_view(center_x, center_y, zoom);
				}
				break;
			default:
				break;
		}
	});

	display.register_mouse_handler([&](int x, int y)
	{
		const int prev_x = mouse_x;
		const int prev_y = mouse_y;

		mouse_x = x;
		mouse_y = display.height() - y;

		renderer.screen_to_world(mouse_x, mouse_y, mouse_world_x, mouse_world_y);

		if (drag_mode == DragMode::pan)
		{
			center_x -= mouse_x - prev_x;
			center_y -= mouse_y - prev_y;
			renderer.set_view(center_x, center_y, zoom);
		}
		else if (drag_mode == DragMode::drag)
		{
			atlasizer.continue_drag(mouse_world_x, mouse_world_y);
		}
	});

	bool quit = false;
	display.register_window_handler([&quit](win::WindowEvent event)
	{
		if (event == win::WindowEvent::close)
			quit = true;
	});

	while (!quit)
	{
		display.process();

		renderer.start_render();

		// draw guides
		renderer.render(win::Color<unsigned char>(0, 255, 0, 255), -1, -1, 1, 400);
		renderer.render(win::Color<unsigned char>(0, 255, 0, 255), -1, -1, 400, 1);

		// items
		for (const auto &item : atlasizer.get_items())
			renderer.render(item.texture, item.x, item.y);

		renderer.draw_text("Atlasizer super alpha v0.000069", 5, display.height() - 15.0f);

		display.swap();
	}
}
