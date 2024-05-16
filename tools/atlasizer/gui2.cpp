#include <win/Display.hpp>
#include <win/AssetRoll.hpp>
#include <win/FileReadStream.hpp>

#include "ZenityDialogManager.hpp"
#include "Renderer.hpp"
#include "Atlasizer.hpp"
#include "LayoutExporter.hpp"

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

	ZenityDialogManager dialog;
	win::AssetRoll roll("atlasizer.roll");
	Renderer renderer(roll, display.width(), display.height());
	Atlasizer atlasizer;

	// interface state
	enum DragMode { none, pan, drag } drag_mode = DragMode::none;
	int center_x = (display.width() / 2.0f) - 75, center_y = (display.height() / 2.0f) - 75;
	float zoom = 1.0f;
	int mouse_x = 0, mouse_y = 0, mouse_world_x = 0, mouse_world_y = 0;
	bool solidmode = false;
	bool snapmode = false;
	std::filesystem::path current_save_file;

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
			case win::Button::lshift:
				solidmode = press;
				break;
			case win::Button::lctrl:
				snapmode = press;
				break;
			case win::Button::d0: case win::Button::d1: case win::Button::d2: case win::Button::d3: case win::Button::d4:
			case win::Button::d5: case win::Button::d6: case win::Button::d7: case win::Button::d8: case win::Button::d9:
				atlasizer.set_padding((int)button - (int)win::Button::d0);
				break;
			default:
				break;
		}
	});

	display.register_character_handler([&](char c)
	{
		switch (c)
		{
			case 'A':
			case 'a':
			{
				const auto &result = dialog.import_image();
				if (result.has_value())
				{
					for (const auto &f: result.value())
					{
						const win::Targa tga(win::Stream(new win::FileReadStream(f)));
						atlasizer.add(renderer.add_texture(tga), f, -1, -1, tga.width(), tga.height());
					}
				}
				break;
			}
			case 'I':
			case 'i':
			{
				const auto &result = dialog.import_layout();
				if (result.has_value())
				{
					{
						// reset everything
						std::vector<int> ids;
						for (auto &item: atlasizer.get_items())
						{
							renderer.remove_texture(item.texture);
							ids.push_back(item.id);
						}

						for (auto id: ids)
							atlasizer.remove(id);
					}

					int padding;
					const auto layout = LayoutExporter::import(result.value(), padding);
					for (const auto &item : layout)
					{
						const win::Targa tga(win::Stream(new win::FileReadStream(item.filename)));
						atlasizer.add(renderer.add_texture(tga), item.filename, item.x, item.y, tga.width(), tga.height());
					}

					atlasizer.set_padding(padding);
				}
				break;
			}
			case 'E':
			case 'e':
			{
				const auto &result = dialog.export_layout();
				if (result.has_value())
				{
					current_save_file = result.value();
					LayoutExporter exporter(current_save_file, atlasizer.get_padding());

					for (const auto &item : atlasizer.get_items())
					{
						AtlasItemDescriptor aid;
						aid.filename = item.texturepath;
						aid.x = item.x;
						aid.y = item.y;
						aid.width = item.w;
						aid.height = item.h;

						exporter.add(aid);
					}

					exporter.save();
				}
				break;
			}
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
			atlasizer.continue_drag(mouse_world_x, mouse_world_y, snapmode);
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
		{
			if (item.valid)
				if (solidmode)
					renderer.render(win::Color<unsigned char>(0, 100, 0, 255), item.x, item.y, item.w, item.h);
				else
					renderer.render(item.texture, item.x, item.y);
			else
				if (solidmode)
					renderer.render(win::Color<unsigned char>(100, 0, 0, 255), item.x, item.y, item.w, item.h);
				else
					renderer.render(item.texture, win::Color<unsigned char>(100, 0, 0, 0), item.x, item.y);
		}

		renderer.draw_text("Atlasizer super alpha v0.000069", 5, display.height() - 15.0f);

		display.swap();
	}
}
