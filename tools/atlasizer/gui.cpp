#include <thread>

#include <win/Display.hpp>
#include <win/AssetRoll.hpp>
#include <win/FileReadStream.hpp>

#include "ControlPanel.hpp"
#include "ListPanel.hpp"
#include "Platform.hpp"
#include "LinuxPlatform.hpp"
#include "WindowsPlatform.hpp"
#include "FilePickerManager.hpp"
#include "Renderer.hpp"
#include "Atlasizer.hpp"
#include "LayoutExporter.hpp"

static Platform &get_platform()
{
#if defined WINPLAT_LINUX
	static LinuxPlatform platform;
#elif defined WINPLAT_WINDOWS
	static WindowsPlatform platform;
#else
#error "unimplemented"
#endif

	return platform;
}

static void get_platform_directories(const Platform &platform, std::filesystem::path &filepicker_state, std::filesystem::path &default_dir)
{
#if defined WINPLAT_LINUX
	filepicker_state = platform.expand_env("$HOME/.atlasizer-defaults");
	default_dir = platform.expand_env("$HOME");
#elif defined WINPLAT_WINDOWS
	filepicker_state = platform.expand_env("%userprofile%\\.atlasizer-defaults.txt");
	default_dir = platform.expand_env("%userprofile%\\desktop");
#else
#error "unimplemented"
#endif
}

static bool in_box(int p_x, int p_y, const win::Box<int> &box)
{
	return p_x >= box.x && p_x <= box.x + box.width && p_y >= box.y && p_y <= box.y + box.height;
}

void gui()
{
	win::DisplayOptions options;
	options.width = 1400;
	options.height = 800;
	options.gl_major = 4;
	options.gl_minor = 4;
	options.caption = "Atlasizer";

	win::Display display(options);
	win::load_gl_functions();

	const Platform &platform = get_platform();

	std::filesystem::path filepicker_state, default_dir;
	get_platform_directories(platform, filepicker_state, default_dir);

	FilePickerManager filepicker(platform, filepicker_state, default_dir);
	win::AssetRoll roll((platform.get_exe_path().parent_path() / "atlasizer.roll").string().c_str());
	Renderer renderer(roll, display.width(), display.height());
	Atlasizer atlasizer;

	win::Box<int> cpanel_box(1, 1, display.width() - 2, 50 - 2);
	ControlPanel cpanel(renderer, cpanel_box);
	cpanel.enable_remove(false);
	cpanel.enable_reload(false);
	cpanel.enable_move_up(false);
	cpanel.enable_move_down(false);

	win::Box<int> lpanel_box(1, 50, 200, display.height() - 51);
	ListPanel lpanel(renderer, lpanel_box);

	// interface state
	enum DragMode { none, pan, drag } drag_mode = DragMode::none;
	int center_x = (display.width() / 2.0f) - 250, center_y = (display.height() / 2.0f) - 105;
	win::Dimensions<int> canvas_dimensions;
	std::optional<std::filesystem::path> current_save_file;
	auto last_interaction = std::chrono::high_resolution_clock::now();
	float zoom = 1.0f;
	int mouse_x = 0, mouse_y = 0, mouse_world_x = 0, mouse_world_y = 0;
	bool solidmode = false;
	bool snapmode = false;
	int selection_id = -1;
	bool dirty = false;

	auto recalculate_statistics = [&]()
	{
		int maxx = 0.0f, maxy = 0.0f;
		for (const auto item : atlasizer.get_items_layout_order())
		{
			maxx = std::max(maxx, item->x + item->w);
			maxy = std::max(maxy, item->y + item->h);
		}

		canvas_dimensions.width = maxx;
		canvas_dimensions.height = maxy;

		const int size = maxx * maxy * 4;
		const float megabytes = size / 1024.0f / 1024.0f;

		char buf[100];
		snprintf(buf, sizeof(buf), "%dx%d (%.1fMB)", maxx, maxy, megabytes);
		cpanel.set_status(buf);
	};

	cpanel.on_import([&]()
	{
		if (dirty && !platform.ask("Discard unsaved changes", "Discard unsaved changes in current layout?"))
			return;

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
				lpanel.add(id, item.filename.filename().string());
			}

			atlasizer.set_padding(padding);
			cpanel.set_pad(padding);

			if (!layout.empty())
				cpanel.enable_reload(true);

			recalculate_statistics();
			dirty = false;
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

		dirty = false;
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
				lpanel.add(id, f.filename().string());
			}

			lpanel.set_selection(-1);

			recalculate_statistics();
			dirty = true;
		}

		cpanel.enable_reload(true);
	});

	cpanel.on_remove([&]()
	{
		const auto items = atlasizer.get_items_layout_order();

		if (items.size() == 1)
			cpanel.enable_reload(false);

		for (const auto &item : items)
		{
			if (item->id == selection_id)
			{
				renderer.remove_texture(item->texture);
				atlasizer.remove(selection_id);
				lpanel.remove(selection_id);
				lpanel.set_selection(-1);
				recalculate_statistics();
				dirty = true;
				return;
			}
		}

		win::bug("No atlas item with id " + std::to_string(selection_id));
	});

	cpanel.on_reload([&]()
	{
		for (auto item : atlasizer.get_items_layout_order())
		{
			const win::Targa tga(win::Stream(new win::FileReadStream(item->texturepath)));

			if (tga.width() != item->w || tga.height() != item->h)
				dirty = true;

			// read the associated texture from disk again
			renderer.remove_texture(item->texture);
			item->texture = renderer.add_texture(tga);
			item->w = tga.width();
			item->h = tga.height();
		}

		atlasizer.check_validity();
	});

	cpanel.on_padding_up([&]()
	{
		const int p = atlasizer.get_padding() + 1;
		atlasizer.set_padding(p);
		cpanel.set_pad(p);

		dirty = true;
	});

	cpanel.on_padding_down([&]()
	{
		const int p = atlasizer.get_padding() - 1;
		if (p >= 0)
		{
			atlasizer.set_padding(p);
			cpanel.set_pad(p);
			dirty = true;
		}
	});

	cpanel.on_move_up([&]()
	{
		atlasizer.move_up(selection_id);

		lpanel.clear();
		for (const auto item : atlasizer.get_items_layout_order())
			lpanel.add(item->id, item->texturepath.filename().string());

		// allow the on_select to decide movement button state
		lpanel.set_selection(selection_id);

		dirty = true;
	});

	cpanel.on_move_down([&]()
	{
		atlasizer.move_down(selection_id);

		lpanel.clear();
		for (const auto item : atlasizer.get_items_layout_order())
			lpanel.add(item->id, item->texturepath.filename().string());

		// allow the on_select to decide movement button state
		lpanel.set_selection(selection_id);

		dirty = true;
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
		last_interaction = std::chrono::high_resolution_clock::now();

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

					if (zoom < 0.5f)
						zoom = 0.5f;
				}
				break;
			case win::Button::lshift:
				solidmode = press;
				break;
			case win::Button::lctrl:
				snapmode = press;
				break;
			default:
				break;
		}
	});

	display.register_mouse_handler([&](int x, int y)
	{
		last_interaction = std::chrono::high_resolution_clock::now();

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
			if (selection_id != -1)
			{
				recalculate_statistics();
				dirty = true;
			}
		}
	});

	recalculate_statistics();

	bool quit = false;
	display.register_window_handler([&](win::WindowEvent event)
	{
		if (event == win::WindowEvent::close)
		{
			if (dirty && !platform.ask("Discard unsaved changes", "Discard unsaved changes in current layout?"))
				return;

			quit = true;
		}
	});

	while (!quit)
	{
		// if no activity in last 10 seconds, slow down this loop
		if (std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - last_interaction).count() > 10000)
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		display.process();

		renderer.start_render();

		// draw guides
		const auto guide_color = dirty ? win::Color<unsigned char>(255, 0, 0, 255) : win::Color<unsigned char>(0, 255, 0, 255);
		renderer.render(guide_color, -1, -1, 1, 400);
		renderer.render(guide_color, -1, -1, 400, 1);

		// draw bounding box
		renderer.render(win::Color<unsigned char>(255, 255, 255, 5), 0, 0, canvas_dimensions.width, canvas_dimensions.height);

		// draw items
		const auto items = atlasizer.get_items_display_order();
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
