#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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
	Renderer renderer(roll, options.width, options.height);
	Atlasizer atlasizer;

	// interface state
	enum DragMode { none, pan, drag } drag_mode = DragMode::none;
	int center_x = -150, center_y = -150;
	float zoom = 1.0f;
	int mouse_x = 0, mouse_y = 0;

	renderer.set_view(center_x, center_y, zoom);

	display.register_button_handler([&](const win::Button button, const bool press)
	{
		switch (button)
		{
			case win::Button::a:
				if (press)
				{
					win::Targa tga(win::Stream(new win::FileReadStream("/home/josh/programming/darktimes/asset/texture/chair1.tga")));
					atlasizer.add(renderer.add_texture(tga), tga.width(), tga.height());
					break;
				}
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
		const auto ident = glm::identity<glm::mat4>();
		const auto view = glm::translate(ident, glm::vec3((float)center_x, (float)center_y, 0.0f));

		const int prev_x = mouse_x;
		const int prev_y = mouse_y;

		mouse_x = x;
		mouse_y = options.height - y;

		if (drag_mode == DragMode::pan)
		{
			center_x -= mouse_x - prev_x;
			center_y -= mouse_y - prev_y;
		}

		renderer.set_view(center_x, center_y, zoom);
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

		for (const auto &item : atlasizer.get_items())
			renderer.render(item.texture, item.x, item.y);

		renderer.draw_text("Atlasizer super alpha v0.000069", 250, 580);

		display.swap();
	}
}
