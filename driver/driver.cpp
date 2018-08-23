#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <cmath>

#include "../win/win.h"

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

int main()
{
	win::system system;
	win::display display = system.make_display("window caption", 800, 600);
	display.cursor(false);

	win::roll roll = "/home/josh/win/driver/assets.roll";

	win::audio_engine audio_engine = display.make_audio_engine(sound_config);
	audio_engine.import(roll.all("ogg"));

	win::font_renderer font_renderer = display.make_font_renderer(display.width(), display.height(), -4.0f, 4.0f, 3.0f, -3.0f);
	win::font font1 = font_renderer.make_font(roll["/usr/share/fonts/noto/NotoSansMono-Regular.ttf"], 0.3f);

	std::cerr << "width is " << display.width() << " and height is " << display.height() << std::endl;
	std::cerr << "screen width is " << win::display::screen_width() << " and screen height is " << win::display::screen_height() << std::endl;

	const float verts[] =
	{
		-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f
	};
	const unsigned int indices[] =
	{
		0, 1, 2, 0, 2, 3
	};

	win::program program = win::load_shaders(roll["vertex.vert"], roll["fragment.frag"]);
	int uniform_projection, uniform_size;
	float ortho_matrix[16];
	win::init_ortho(ortho_matrix, -4.0f, 4.0f, 3.0f, -3.0f);
	uniform_projection = glGetUniformLocation(program, "projection");
	uniform_size = glGetUniformLocation(program, "size");
	glUniformMatrix4fv(uniform_projection, 1, false, ortho_matrix);
	glUniform1f(uniform_size, Block::SIZE);

	win::vao vao;

	// element buffer
	win::ebo ebo;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// vertex buffer
	win::vbo vbo_vertex;
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
	glEnableVertexAttribArray(0);

	// position buffer
	win::vbo vbo_position;
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, NULL);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	// color buffer
	win::vbo vbo_color;
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
	display.event_button([&quit, &paused, &audio_engine](win::button button, bool press)
	{
		if(press)
			std::cerr << "key: " << win::key_name(button) << std::endl;
		if(press && button == win::button::ESC)
			quit = true;
		else if(press && button == win::button::SPACE)
		{
			paused = !paused;
			if(paused)
				audio_engine.pause();
			else
				audio_engine.resume();
		}
		else if(press && button == win::button::LALT)
			audio_engine.listener(0, 0);
		else if(press)
			audio_engine.play(1);
	});

	// display.event_joystick([](win::joystick_axis axis, int value)
	// {
	// 	switch(axis)
	// 	{
	// 		case win::joystick_axis::RIGHT_X:
	// 			std::cerr << "axis right x, value " << value << std::endl;
	// 			break;
	// 		case win::joystick_axis::RIGHT_Y:
	// 			std::cerr << "axis right y, value " << value << std::endl;
	// 			break;
	// 		case win::joystick_axis::LEFT_X:
	// 			std::cerr << "axis left x, value " << value << std::endl;
	// 			break;
	// 		case win::joystick_axis::LEFT_Y:
	// 			std::cerr << "axis left y, value " << value << std::endl;
	// 			break;
	// 		case win::joystick_axis::RIGHT_TRIGGER:
	// 			std::cerr << "axis right trigger, value " << value << std::endl;
	// 			break;
	// 		case win::joystick_axis::LEFT_TRIGGER:
	// 			std::cerr << "axis left trigger, value " << value << std::endl;
	// 			break;
	// 	}
	// });

	// display.event_mouse([&quit](int x, int y)
	// {
	// 	fprintf(stderr, "x: %d, y: %d\n", x, y);
	// });

	const int block_sid = audio_engine.play(0, block.x, block.y);
	for(;;)
	{
		auto start = std::chrono::high_resolution_clock::now();

		if(!display.process() || quit)
			break;

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
			block_color[1] = 0.0f;
			block_color[2] = 0.0f;

			glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(block_position), block_position, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
			glBufferData(GL_ARRAY_BUFFER, sizeof(block_color), block_color, GL_DYNAMIC_DRAW);

			glClear(GL_COLOR_BUFFER_BIT);
			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, 1);
		}

		const time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		char formatted[50];
		if(0 == strftime(formatted, sizeof(formatted), "Today is %A, %B %d\n%I:%M:%S %p", tm))
			strcpy(formatted, "null");

		font_renderer.draw(font1, formatted, 0.0f, -2.9f, win::color(1.0f, 1.0f, 0.0f), win::font_renderer::CENTERED);
		glBindVertexArray(vao);
		glUseProgram(program);

		display.swap();

		std::this_thread::sleep_for(std::chrono::milliseconds(4));
		while(std::chrono::duration<float, std::micro>(std::chrono::high_resolution_clock::now() - start).count() < 16666.0f);
	}

	return 0;
}
