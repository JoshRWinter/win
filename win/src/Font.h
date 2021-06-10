#ifndef WIN_FONT_H
#define WIN_FONT_H

#include <array>
#include <memory>

namespace win
{

class FontRenderer;

struct metric
{
	float advance;
	float bearing_y;
	float bitmap_left;
};

class Font
{
	friend class FontRenderer;
public:
	Font() = default;
	Font(const FontRenderer &parent, AssetRollStream, float);
	Font(const Font&) = delete;
	Font(Font&&) = default;

	void operator=(Font&) = delete;
	Font &operator=(Font&&) = default;

	float size() const;

private:
	Texture atlas;
	std::array<metric, 95> metrics;
	float box_width; // width of each tile in the atlas
	float box_height; // height of each tile in the atlas
	float max_bearing_y; // greatest y bearing
	float vertical; // vertical advance
	float fontsize;
};

class FontRenderer
{
	friend class Font;
public:
	FontRenderer();
	FontRenderer(int, int, float, float, float, float);
	FontRenderer(const FontRenderer&) = delete;
	FontRenderer(FontRenderer&&);

	void operator=(const FontRenderer&) = delete;
	FontRenderer &operator=(FontRenderer&&);

	void draw(const Font&, const char *, float, float, const Color&, bool centered = false);

private:
	float line_length(const Font&, const char*, int) const;

	// renderer settings
	int display_width, display_height;
	float left, right, bottom, top;

	// opengl objects
	Program program;
	Vao vao;
	Vbo vbo_vertex, vbo_position, vbo_texcoord;
	Vbo ebo;
	int uniform_size, uniform_color;

	std::vector<float> pos_buffer;
	std::vector<unsigned short> texcoord_buffer;
};

}

#endif