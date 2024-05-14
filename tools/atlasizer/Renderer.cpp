#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <win/gl/GL.hpp>
#include <win/AssetRoll.hpp>

#include "Renderer.hpp"

using namespace win::gl;

int Renderer::next_id = 0;

static GLint get_uniform(const win::GLProgram &program, const char *name)
{
	const auto loc = glGetUniformLocation(program.get(), name);

	if (loc == -1)
		win::bug("No uniform " + std::string(name));

	return loc;
}

Renderer::Renderer(win::AssetRoll &roll, int viewport_width, int viewport_height)
	: viewport_width(viewport_width)
	, viewport_height(viewport_height)
	, projection(glm::ortho((float)0, (float)viewport_width, (float)0, (float)viewport_height))
	, view(glm::identity<glm::mat4>())
	, font(win::Dimensions<int>(viewport_width, viewport_height), win::Area<float>(0, viewport_width, 0, viewport_height), 12, roll["NotoSans-Regular.ttf"])
	, text_renderer(win::Dimensions<int>(viewport_width, viewport_height), win::Area<float>(0, viewport_width, 0, viewport_height), GL_TEXTURE1, true)
{
	set_view(viewport_width / 2.0f, viewport_height / 2.0f, 1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	program = win::GLProgram(win::load_gl_shaders(roll["vertex_shader.vert"], roll["fragment_shader.frag"]));
	glUseProgram(program.get());

	uniform_mvp = get_uniform(program, "mvp");
	uniform_use_texture = get_uniform(program, "use_texture");
	uniform_use_color = get_uniform(program, "use_color");
	uniform_color = get_uniform(program, "color");

	const float verts[]
	{
		-0.5f, 0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.0f, 1.0f,
		0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 1.0f, 1.0f
	};

	glBindVertexArray(vao.get());
	glBindBuffer(GL_ARRAY_BUFFER, vbo.get());

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void Renderer::screen_to_world(int mouse_x, int mouse_y, int &world_x, int &world_y) const
{
	const glm::vec4 ndc((mouse_x / (viewport_width / 2.0f)) - 1.0f, (mouse_y / (viewport_height / 2.0f)) - 1.0f, -1, 1);
	const auto vp = projection * view;
	const auto ivp = glm::inverse(vp);
	const auto point = ivp * ndc;

	world_x = std::roundf(point.x);
	world_y = std::roundf(point.y);
}

int Renderer::add_texture(win::Targa &tga)
{
	const int key = next_id++;

	auto &tex = texture_map[key];
	tex.w = tga.width();
	tex.h = tga.height();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex.tex.get());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tga.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	win::gl_check_error();

	return key;
}

void Renderer::start_render()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::render(int texture, int x, int y)
{
	const auto &tex = texture_map[texture];

	render(&tex, NULL, x, y, tex.w, tex.h);
}

void Renderer::render(win::Color<unsigned char> rgba, int x, int y, int w, int h)
{
	render(NULL, &rgba, x, y, w, h);
}

void Renderer::render(int texture, win::Color<unsigned char> rgba, int x, int y)
{
	const auto &tex = texture_map[texture];

	render(&tex, &rgba, x, y, tex.w, tex.h);
}

void Renderer::set_view(int centerx, int centery, float zoom)
{
	const auto ident = glm::identity<glm::mat4>();
	const auto translate = glm::translate(ident, glm::vec3((float)-(centerx - (viewport_width / 2.0f)), (float)-(centery - (viewport_height / 2.0f)), 0.0f));
	const auto scale = glm::scale(ident, glm::vec3(zoom, zoom, 1.0f));

	view = translate * scale;
}

void Renderer::draw_text(const char *msg, int x, int y)
{
	draw_text(msg, x, y, win::Color<float>(1.0f, 1.0f, 1.0f, 1.0f));
}

void Renderer::draw_text(const char *msg, int x, int y, const win::Color<float> &color)
{
	text_renderer.draw(font, msg, x, y, color);
	text_renderer.flush();
}

void Renderer::render(const Texture *texture, const win::Color<unsigned char> *color, int x, int y, int w, int h)
{
	const auto ident = glm::identity<glm::mat4>();
	const auto translate = glm::translate(ident, glm::vec3(x + (w / 2.0f), y + (h / 2.0f), 0.0f));
	const auto scale = glm::scale(ident, glm::vec3(w, h, 1.0f));

	const auto mvp = projection * view * translate * scale;

	glUseProgram(program.get());
	glBindVertexArray(vao.get());

	glUniform1i(uniform_use_texture, texture != NULL);
	glUniform1i(uniform_use_color, color != NULL);

	if (color != NULL)
		glUniform4f(uniform_color, color->red / (float)std::numeric_limits<unsigned char>::max(), (float)color->green / std::numeric_limits<unsigned char>::max(), (float)color->blue / std::numeric_limits<unsigned char>::max(), (float)color->alpha / std::numeric_limits<unsigned char>::max());

	if (texture != NULL)
		glBindTexture(GL_TEXTURE_2D, texture->tex.get());

	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	win::gl_check_error();
}
