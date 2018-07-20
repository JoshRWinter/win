#ifndef WIN_FONT_H
#define WIN_FONT_H

#include <array>

namespace win
{

class font_renderer;

class font
{
	friend font_renderer;

	struct kernvector { float advance, bitmap_left; };

public:
	font(const font&) = delete;
	font(font&&);
	~font();

	font &operator=(font&&);

private:
	font(const font_renderer &parent, resource&, float);
	void finalize();

	unsigned atlas_;
	std::array<kernvector, 96> kern_;
	float size_;
};

class font_renderer
{
	friend font;
	friend display;

public:
	font_renderer(const font_renderer&) = delete;
	font_renderer(font_renderer&&);
	~font_renderer();

	font_renderer &operator=(const font_renderer&) = delete;
	font_renderer &operator=(font_renderer&&);

	font make_font(resource&, float);

private:
	font_renderer(int, int, float, float, float, float);
	void finalize();

	// renderer settings
	int display_width_, display_height_;
	float left_, right_, bottom_, top_;

	// opengl objects
	unsigned program_;
	unsigned vao_;
	unsigned vbo_vertex_, vbo_position_, vbo_color_, vbo_texcoord_;
	unsigned ebo_;
};

}

#endif