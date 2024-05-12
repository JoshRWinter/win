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

	bool quit = false;
	win::AssetRoll roll("atlasizer.roll");
	win::load_gl_functions();
	Renderer renderer(roll, options.width, options.height);
	Atlasizer atlasizer;

	display.register_button_handler([&renderer, &atlasizer](const win::Button button, const bool press)
	{
		if (!press)
			return;

		switch (button)
		{
			case win::Button::a:
			{
				win::Targa tga(win::Stream(new win::FileReadStream("/home/josh/programming/darktimes/asset/texture/chair1.tga")));
				const int texture = renderer.add_texture(tga);
				atlasizer.add(texture, tga.width(), tga.height());
				break;
			}
			default: break;
		}
	});

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
