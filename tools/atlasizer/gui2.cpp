#include <win/Display.hpp>
#include <win/AssetRoll.hpp>
#include <win/FileReadStream.hpp>

#include "ControlPanel.hpp"
#include "ListPanel.hpp"
#include "Platform.hpp"
#include "LinuxPlatform.hpp"
#include "FilePickerManager.hpp"
#include "Renderer.hpp"
#include "Atlasizer.hpp"
#include "LayoutExporter.hpp"

static Platform &get_platform()
{
#ifdef WINPLAT_LINUX
	static LinuxPlatform platform;
	return platform;
#else
#error "unimplemented"
#endif
}

static bool in_box(int p_x, int p_y, const win::Box<int> &box)
{
	return p_x >= box.x && p_x <= box.x + box.width && p_y >= box.y && p_y <= box.y + box.height;
}

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

	const Platform &platform = get_platform();
	FilePickerManager filepicker(platform, platform.expand_env("$HOME/.atlasizer-defaults"), platform.expand_env("$HOME"));
	win::AssetRoll roll("atlasizer.roll");
	Renderer renderer(roll, display.width(), display.height());
	Atlasizer atlasizer;

	win::Box<int> cpanel_box(1, 1, display.width() - 2, 50 - 2);
	ControlPanel cpanel(renderer, cpanel_box);
	cpanel.enable_remove(false);
	cpanel.enable_move_up(false);
	cpanel.enable_move_down(false);

	win::Box<int> lpanel_box(1, 50, 200, display.height() - 51);
	ListPanel lpanel(renderer, lpanel_box);

	// interface state
	enum DragMode { none, pan, drag } drag_mode = DragMode::none;
	int center_x = (display.width() / 2.0f) - 75, center_y = (display.height() / 2.0f) - 105;
	float zoom = 1.0f;
	int mouse_x = 0, mouse_y = 0, mouse_world_x = 0, mouse_world_y = 0;
	bool solidmode = false;
	bool snapmode = false;
	int selection_id = -1;
	std::optional<std::filesystem::path> current_save_file;

	cpanel.on_import([&]()
	{
		const auto &result = filepicker.import_layout();
		if (result.has_value())
		{
			{
				// reset everything
				std::vector<int> ids;
				for (auto &item: atlasizer.get_items_layout_order())
				{
					renderer.remove_texture(item->texture);
					ids.push_back(item->id);
				}

				for (auto id: ids)
					atlasizer.remove(id);
			}

			lpanel.clear();
			lpanel.set_selection(-1);

			current_save_file = result.value();

			int padding;
			const auto layout = LayoutExporter::import(result.value(), padding);
			for (const auto &item : layout)
			{
				const win::Targa tga(win::Stream(new win::FileReadStream(item.filename)));
				const int id = atlasizer.add(renderer.add_texture(tga), item.filename, item.x, item.y, tga.width(), tga.height());
				lpanel.add(id, item.filename.filename());
			}

			atlasizer.set_padding(padding);
			cpanel.set_pad(padding);
		}
	});

	cpanel.on_export([&]()
	{
		std::filesystem::path savefile;

		if (current_save_file.has_value())
		{
			savefile = current_save_file.value();
		}
		else
		{
			const auto result = filepicker.export_layout();
			if (result.has_value())
			{
				savefile = result.value();
				current_save_file = result.value();
			}
			else return;
		}

		LayoutExporter exporter(savefile, atlasizer.get_padding());

		for (const auto &item : atlasizer.get_items_layout_order())
		{
			AtlasItemDescriptor aid;
			aid.filename = item->texturepath;
			aid.x = item->x;
			aid.y = item->y;
			aid.width = item->w;
			aid.height = item->h;

			exporter.add(aid);
		}

		exporter.save();
	});

	cpanel.on_add([&]()
	{
		const auto &result = filepicker.import_image();
		if (result.has_value())
		{
			for (const auto &f: result.value())
			{
				const win::Targa tga(win::Stream(new win::FileReadStream(f)));
				const int id = atlasizer.add(renderer.add_texture(tga), f, -1, -1, tga.width(), tga.height());
				lpanel.add(id, f.filename());
			}

			lpanel.set_selection(-1);
		}
	});

	cpanel.on_remove([&]()
	{
		const auto items = atlasizer.get_items_layout_order();
		for (const auto &item : items)
		{
			if (item->id == selection_id)
			{
				renderer.remove_texture(item->texture);
				atlasizer.remove(selection_id);
				lpanel.remove(selection_id);
				lpanel.set_selection(-1);
				return;
			}
		}

		win::bug("No atlas item with id " + std::to_string(selection_id));
	});

	cpanel.on_padding_up([&]()
	{
		const int p = atlasizer.get_padding() + 1;
		atlasizer.set_padding(p);
		cpanel.set_pad(p);
	});

	cpanel.on_padding_down([&]()
	{
		const int p = atlasizer.get_padding() - 1;
		if (p >= 0)
		{
			atlasizer.set_padding(p);
			cpanel.set_pad(p);
		}
	});

	cpanel.on_move_up([&]()
	{
		atlasizer.move_up(selection_id);

		lpanel.clear();
		for (const auto item : atlasizer.get_items_layout_order())
			lpanel.add(item->id, item->texturepath.filename());

		// allow the on_select to decide movement button state
		lpanel.set_selection(selection_id);
	});

	cpanel.on_move_down([&]()
	{
		atlasizer.move_down(selection_id);

		lpanel.clear();
		for (const auto item : atlasizer.get_items_layout_order())
			lpanel.add(item->id, item->texturepath.filename());

		// allow the on_select to decide movement button state
		lpanel.set_selection(selection_id);
	});

	lpanel.on_select([&](int id)
	{
		selection_id = id;

		cpanel.enable_remove(id != -1);

		const auto &items = atlasizer.get_items_layout_order();

		if (!items.empty())
		{
			const auto first = *items.begin();
			const auto last = *(items.end() - 1);

			cpanel.enable_move_up(id != -1 && first->id != id);
			cpanel.enable_move_down(id != -1 && last->id != id);
		}
		else
		{
			cpanel.enable_move_up(false);
			cpanel.enable_move_down(false);
		}
	});

	display.register_button_handler([&](const win::Button button, const bool press)
	{
		switch (button)
		{
			case win::Button::mouse_right:
				if (press)
				{
					if (!in_box(mouse_x, mouse_y, cpanel_box) && !in_box(mouse_x, mouse_y, lpanel_box) && drag_mode == DragMode::none)
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
					if (in_box(mouse_x, mouse_y, cpanel_box))
						cpanel.click(true);
					else if (in_box(mouse_x, mouse_y, lpanel_box))
						lpanel.click(true);
					else if (drag_mode == DragMode::none)
					{
						drag_mode = DragMode::drag;
						selection_id = atlasizer.start_drag(mouse_world_x, mouse_world_y);
						lpanel.set_selection(selection_id);
					}
				}
				else
				{
					if (drag_mode == DragMode::none)
					{
						cpanel.click(false);
						lpanel.click(false);
					}
					else
						drag_mode = DragMode::none;
				}
				break;
			case win::Button::num_plus:
				if (drag_mode == DragMode::none && press)
				{
					zoom += 0.1f;
				}
				break;
			case win::Button::num_minus:
				if (drag_mode == DragMode::none && press)
				{
					zoom -= 0.1f;
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

	display.register_mouse_handler([&](int x, int y)
	{
		const int prev_x = mouse_x;
		const int prev_y = mouse_y;

		mouse_x = x;
		mouse_y = display.height() - y;

		cpanel.mouse_move(mouse_x, mouse_y);
		lpanel.mouse_move(mouse_x, mouse_y);

		renderer.screen_to_world(mouse_x, mouse_y, mouse_world_x, mouse_world_y);

		if (drag_mode == DragMode::pan)
		{
			int prev_world_x, prev_world_y;
			renderer.screen_to_world(prev_x, prev_y, prev_world_x, prev_world_y);

			center_x -= mouse_world_x - prev_world_x;
			center_y -= mouse_world_y - prev_world_y;
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

		const auto items = atlasizer.get_items_display_order();

		// draw bounding box
		int max_x = 0, max_y = 0;
		for (const auto &item : items)
		{
			max_x = std::max(max_x, item->x + item->w);
			max_y = std::max(max_y, item->y + item->h);
		}
		renderer.render(win::Color<unsigned char>(255, 255, 255, 5), 0, 0, max_x, max_y);

		// items
		for (const auto &item : items)
		{
			// sweet jebus
			if (item->valid)
				if (solidmode)
					if (selection_id == item->id)
						renderer.render(win::Color<unsigned char>(50, 100, 50, 255), item->x, item->y, item->w, item->h);
					else
						renderer.render(win::Color<unsigned char>(0, 100, 0, 255), item->x, item->y, item->w, item->h);
				else
					if (selection_id == item->id)
						renderer.render(item->texture, win::Color<unsigned char>(20, 20, 20, 0), item->x, item->y);
					else
						renderer.render(item->texture, item->x, item->y);
			else
				if (solidmode)
					if (selection_id == item->id)
						renderer.render(win::Color<unsigned char>(100, 50, 50, 255), item->x, item->y, item->w, item->h);
					else
						renderer.render(win::Color<unsigned char>(100, 0, 0, 255), item->x, item->y, item->w, item->h);
				else
					if (selection_id == item->id)
						renderer.render(item->texture, win::Color<unsigned char>(100, 50, 50, 0), item->x, item->y);
					else
						renderer.render(item->texture, win::Color<unsigned char>(100, 0, 0, 0), item->x, item->y);
		}

		// draw gui panels
		renderer.set_view(display.width() / 2, display.height() / 2, 1.0f);
		cpanel.draw();
		lpanel.draw();
		renderer.set_view(center_x, center_y, zoom);

		display.swap();
	}
}
