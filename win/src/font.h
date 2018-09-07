#ifndef WIN_FONT_H
#define WIN_FONT_H

#include <array>

namespace win
{

class font_renderer;

struct metric
{
	float advance;
	float bearing_y;
	float bitmap_left;
};

struct font
{
	friend font_renderer;

	font();
	font(const font&) = delete;
	font(font&&);
	~font();

	void operator=(font&) = delete;
	font &operator=(font&&);

	unsigned atlas;
	std::array<metric, 95> metrics;
	float box_width; // width of each tile in the atlas
	float box_height; // height of each tile in the atlas
	float max_bearing_y; // greatest y bearing
	float vertical; // vertical advance

private:
	font(const font_renderer &parent, data, float);
	void move(font&);
	void finalize();
};

class font_renderer
{
	friend font;
	friend display;

public:
	static constexpr int CENTERED = 1;

	font_renderer();
	font_renderer(const font_renderer&) = delete;
	font_renderer(font_renderer&&);
	~font_renderer();

	font_renderer &operator=(const font_renderer&) = delete;
	font_renderer &operator=(font_renderer&&);

	void draw(const font&, const char *, float, float, const color&, int = 0);

	font make_font(data, float);

private:
	font_renderer(int, int, float, float, float, float);
	void move(font_renderer&);
	float line_length(const font&, const char*, int) const;
	void finalize();

	// renderer settings
	int display_width_, display_height_;
	float left_, right_, bottom_, top_;

	// opengl objects
	unsigned program_;
	unsigned vao_;
	unsigned vbo_vertex_, vbo_position_, vbo_texcoord_;
	unsigned ebo_;
	int uniform_size_, uniform_color_;
};

}

#endif
