#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include <win/Win.hpp>
#include <win/gl/GL.hpp>
#include <win/Targa.hpp>
#include <win/gl/GLTextRenderer.hpp>

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

	int add_texture(win::Targa &tga);
	void start_render();
	void render(int texture, int x, int y);
	void draw_text(const char *msg, int x, int y);
	void draw_text(const char *msg, int x, int y, const win::Color<float> &color);

private:
	glm::mat4 projection;

	std::unordered_map<int, Texture> texture_map;

	int uniform_mvp;

	win::GLVertexArray vao;
	win::GLBuffer vbo;
	win::GLProgram program;

	win::GLFont font;
	win::GLTextRenderer text_renderer;
};
