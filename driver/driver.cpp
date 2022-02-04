#include <chrono>
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <win/win.hpp>
#include <win/display.hpp>
#include <win/assetroll.hpp>
#include <win/atlas.hpp>
#include <win/soundengine.hpp>
#include <win/fontrenderer.hpp>
#include <win/font.hpp>
#include <win/gl.hpp>

extern const char *vertexshader,*fragmentshader;

struct Block
{
	static constexpr float SIZE = 1.0f;
	Block() : x(0.0f), y(0.0f), xv(0.06f), yv(0.0f) {}
	float x, y, xv, yv;
};

/*
static float distance(float x1, float y1, float x2, float y2)
{
	return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));;
}
*/

static void sound_config(float listenerx, float listenery, float sourcex, float sourcey, float *volume, float *balance)
{
	*volume = 1.0f;// - (distance(listenerx, listenery, sourcex, sourcey) / 38.0f);
	*balance = (sourcex - listenerx) / 15.0f;
}

#if defined WINPLAT_WINDOWS && defined NDEBUG
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
	win::DisplayOptions display_options;
	display_options.caption = "window caption";
	display_options.width = 800;
	display_options.height = 600;
	display_options.gl_major = 3;
	display_options.gl_minor = 3;

	win::Display display(display_options);
	display.cursor(true);

#if defined WINPLAT_LINUX
	win::AssetRoll roll("/home/josh/programming/win/driver/assets.roll");
#elif defined WINPLAT_WINDOWS
	win::AssetRoll roll("c:\\users\\josh\\desktop\\win\\driver\\assets.roll");
#endif

	win::Atlas atlas(roll["main.atlas"]);

	auto music = "../../fishtank/assets_local/Motions.ogg";
	auto effect = "../../fishtank/assets_local/platform_destroy.ogg";

	win::SoundEngine audio_engine(display, roll, sound_config);

	win::FontRenderer font_renderer(win::IDimensions2D(display.width(), display.height()), win::FScreenArea(-4.0f, 4.0f, -3.0f, 3.0f));
	win::Font font1(font_renderer, roll["../../fishtank/assets/arial.ttf"], 0.5f);

	std::cerr << "width is " << display.width() << " and height is " << display.height() << std::endl;
	std::cerr << "screen width is " << win::Display::screen_width() << " and screen height is " << win::Display::screen_height() << std::endl;

	const win::AtlasItem coords = atlas.item(3);
	const float verts[] =
	{
		-0.5f, 0.5f, coords.x1, coords.y2,
		-0.5f, -0.5f, coords.x1, coords.y1,
		0.5f, -0.5f, coords.x2, coords.y1,
		0.5f, 0.5f,	coords.x2, coords.y2
	};
	const unsigned int indices[] =
	{
		0, 1, 2, 0, 2, 3
	};

	GLuint program = win::load_gl_shaders(roll["vertex.vert"], roll["fragment.frag"]);
	glUseProgram(program);

	glm::mat4 ortho = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f);
	const int uniform_projection = glGetUniformLocation(program, "projection");
	const int uniform_size = glGetUniformLocation(program, "size");
	glUniformMatrix4fv(uniform_projection, 1, false, glm::value_ptr(ortho));
	glUniform1f(uniform_size, Block::SIZE);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// element buffer
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// vertex buffer
	GLuint vbo_vertex;
	glGenBuffers(1, &vbo_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(3);

	// position buffer
	GLuint vbo_position;
	glGenBuffers(1, &vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, NULL);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	// color buffer
	GLuint vbo_color;
	glGenBuffers(1, &vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, false, 0, NULL);
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);

	Block block;
	float block_position[2];
	unsigned char block_color[3];

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	bool quit = false;
	bool paused = false;
	display.register_button_handler([&effect, &quit, &paused, &audio_engine](win::Button button, bool press)
	{
		if(press)
			std::cerr << "key: " << win::key_name(button) << std::endl;
		if(press && button == win::Button::esc)
			quit = true;
		else if(press)
			audio_engine.play(effect);
	});

	float mousex = 0.0f, mousey = 0.0f;
	display.register_mouse_handler([&mousex, &mousey](int x, int y)
	{
		mousex = ((x / 800.0f) * 8.0f) - 4.0f;
		mousey = -(((y / 600.0f) * 6.0f) - 3.0f);
	});

	display.register_window_handler([&quit](win::WindowEvent event)
	{
		if (event == win::WindowEvent::close)
			quit = true;
	});

	// display.register_character_handle([](int key)
	// {
	// 	std::cerr << (char)key;
	// });

	const int block_sid = audio_engine.play(music, block.x, block.y, true);
	while(!quit)
	{
		auto start = std::chrono::high_resolution_clock::now();
		display.process();

		audio_engine.process();

		// process entities
		{
			block.x += block.xv;
			block.y += block.yv;

			if(block.x + Block::SIZE > 4.0f)
			{
				block.x = 4.0f - Block::SIZE;
				block.xv = -block.xv;
			}
			else if(block.x < -4.0f)
			{
				block.x = -4.0f;
				block.xv = -block.xv;
			}
			if(block.y + Block::SIZE > 3.0f)
			{
				block.y = 3.0f - Block::SIZE;
				block.yv = -block.yv;
			}
			else if(block.y < -3.0f)
			{
				block.y = -3.0f;
				block.yv = -block.yv;
			}

			audio_engine.source(block_sid, block.x, block.y);

			block_position[0] = block.x;
			block_position[1] = block.y;
			block_color[0] = 1.0f;
			block_color[1] = 1.0f;
			block_color[2] = 1.0f;

			glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(block_position), block_position, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(block_color), block_color, GL_DYNAMIC_DRAW);
			glBindTexture(GL_TEXTURE_2D, atlas.texture());

			glClear(GL_COLOR_BUFFER_BIT);
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, 1);
		}

		const time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		char formatted[100];
		if(0 == strftime(formatted, sizeof(formatted), "Today is %A, %B %d\n%I:%M:%S %p", tm))
			strcpy(formatted, "null");

		font_renderer.draw(font1, formatted, mousex, mousey, win::Color(1.0f, 1.0f, 0.0f), true);
		glBindVertexArray(vao);
		glUseProgram(program);

		display.swap();

		std::this_thread::sleep_for(std::chrono::milliseconds(4));
		while(std::chrono::duration<float, std::micro>(std::chrono::high_resolution_clock::now() - start).count() < 16666.0f);
	}

	glDeleteBuffers(1, &vbo_color);
	glDeleteBuffers(1, &vbo_position);
	glDeleteBuffers(1, &vbo_vertex);
	glDeleteBuffers(1, &ebo);

	glDeleteVertexArrays(1, &vao);

	glDeleteShader(program);

	return 0;
}
