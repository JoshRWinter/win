#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include <win/Win.hpp>
#include <win/gl/GL.hpp>
#include <win/Targa.hpp>
#include <win/gl/GLTextRenderer.hpp>
#include <win/AssetRoll.hpp>

struct Texture
{
	win::GLTexture tex;
	int w;
	int h;
};

class Renderer
{
	WIN_NO_COPY_MOVE(Renderer);

public:
	explicit Renderer(win::AssetRoll &roll, int viewport_width, int viewport_height);

	void screen_to_world(int mouse_x, int mouse_y, int &world_x, int &world_y) const;
	int add_texture(const win::Targa &tga);
	void remove_texture(int texture);
	void start_render();
	void render(int texture, int x, int y);
	void render(int texture, win::Color<unsigned char> rgba, int x, int y);
	void render(win::Color<unsigned char> rgba, int x, int y, int w, int h);
	void set_view(int centerx, int centery, float zoom);
	void draw_text(const char *msg, int x, int y, bool centered);
	void draw_text(const char *msg, int x, int y, const win::Color<unsigned char> &color, bool centered);

private:
	void render(const Texture *texture, const win::Color<unsigned char> *color, int x, int y, int w, int h);

	static int next_id;

	int viewport_width, viewport_height;
	glm::mat4 projection, view;

	std::unordered_map<int, Texture> texture_map;

	GLint uniform_mvp, uniform_use_color, uniform_color, uniform_use_texture;

	win::GLVertexArray vao;
	win::GLBuffer vbo;
	win::GLProgram program;

	win::GLFont font;
	win::GLTextRenderer text_renderer;
};
