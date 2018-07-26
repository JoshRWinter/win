#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>

#include "../win/win.h"

extern const char *vertexshader,*fragmentshader;

struct Block
{
	static constexpr float SIZE = 1.0f;
	Block() : x(0.0f), y(0.0f), xv(0.1f), yv(0.0f) {}
	float x, y, xv, yv;
};

int main()
{
	win::system system;
	win::display display = system.make_display("window caption", 800, 600);
	display.cursor(false);

	win::resource rc("/usr/share/fonts/noto/NotoSansMono-Regular.ttf");
	win::font_renderer font_renderer = display.make_font_renderer(display.width(), display.height(), -4.0f, 4.0f, 3.0f, -3.0f);
	win::font font1 = font_renderer.make_font(rc, 0.3f);

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

	unsigned program = win::load_shaders(vertexshader, fragmentshader);
	glUseProgram(program);
	int uniform_projection, uniform_size;
	float ortho_matrix[16];
	win::init_ortho(ortho_matrix, -4.0f, 4.0f, 3.0f, -3.0f);
	uniform_projection = glGetUniformLocation(program, "projection");
	uniform_size = glGetUniformLocation(program, "size");
	glUniformMatrix4fv(uniform_projection, 1, false, ortho_matrix);
	glUniform1f(uniform_size, Block::SIZE);

	unsigned vao;
	unsigned vbo_vertex, vbo_position, vbo_color;
	unsigned ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo_vertex);
	glGenBuffers(1, &vbo_position);
	glGenBuffers(1, &vbo_color);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	// element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
	glEnableVertexAttribArray(0);

	// position buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, NULL);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

	// color buffer
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
	display.event_button([&quit](win::button, bool press)
	{
		if(press)
			quit = true;
	});

	// display.event_mouse([&quit](int x, int y)
	// {
	// 	fprintf(stderr, "x: %d, y: %d\n", x, y);
	// });

	for(;;)
	{
		auto start = std::chrono::high_resolution_clock::now();

		if(!display.process() || quit)
			goto done;

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
done:

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo_color);
	glDeleteBuffers(1, &vbo_position);
	glDeleteBuffers(1, &vbo_vertex);
	glDeleteVertexArrays(1, &vao);

	glDeleteProgram(program);

	return 0;
}

const char *vertexshader =
"#version 330 core\n"
"layout (location = 0) in vec2 verts;"
"layout (location = 1) in vec2 position;"
"layout (location = 2) in vec3 color;"
"uniform mat4 projection;"
"uniform float size;"
"out vec3 fcolor;"
"void main(){"
"mat4 translate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, position.x + (size / 2.0), position.y + (size / 2.0), 0.0, 1.0);"
"fcolor = color;"
"gl_Position = projection * translate * vec4(verts.x * size, verts.y * size, 0.0, 1.0);"
"}"
,*fragmentshader =
"#version 330 core\n"
"in vec3 fcolor;"
"out vec4 color;"
"void main(){"
"color = vec4(fcolor.rgb, 1.0);"
"}"
;
