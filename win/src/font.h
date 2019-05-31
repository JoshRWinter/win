#ifndef WIN_FONT_H
#define WIN_FONT_H

#include <array>
#include <memory>

namespace win
{

class font_renderer;

struct metric
{
	float advance;
	float bearing_y;
	float bitmap_left;
};

struct font_remote;
struct font
{
	friend font_renderer;

	font() = default;;
	font(const font&) = delete;
	font(font&&);
	~font();

	void operator=(font&) = delete;
	font &operator=(font&&);

private:
	font(const font_renderer &parent, data, float);
	void finalize();

	std::unique_ptr<font_remote> remote;
};

struct font_remote
{
	font_remote() = default;
	font_remote(const font_remote&) = delete;
	font_remote(font_remote&&) = delete;
	void operator=(const font_remote&) = delete;
	void operator=(font_remote&&) = delete;

	unsigned atlas;
	std::array<metric, 95> metrics;
	float box_width; // width of each tile in the atlas
	float box_height; // height of each tile in the atlas
	float max_bearing_y; // greatest y bearing
	float vertical; // vertical advance
};

struct font_renderer_remote;
class font_renderer
{
	friend font;
	friend display;

public:
	static constexpr int CENTERED = 1;

	font_renderer() = default;
	font_renderer(const font_renderer&) = delete;
	font_renderer(font_renderer&&);
	~font_renderer();

	font_renderer &operator=(const font_renderer&) = delete;
	font_renderer &operator=(font_renderer&&);

	void draw(const font&, const char *, float, float, const color&, int = 0);

	font make_font(data, float);

private:
	font_renderer(int, int, float, float, float, float);
	float line_length(const font&, const char*, int) const;
	void finalize();

	std::unique_ptr<font_renderer_remote> remote;
};

struct font_renderer_remote
{
	font_renderer_remote() = default;
	font_renderer_remote(const font_renderer_remote&) = delete;
	font_renderer_remote(font_renderer_remote&&) = delete;
	void operator=(const font_renderer_remote&) = delete;
	void operator=(font_renderer_remote&&) = delete;

	// renderer settings
	int display_width_, display_height_;
	float left_, right_, bottom_, top_;

	// opengl objects
	unsigned program_;
	unsigned vao_;
	unsigned vbo_vertex_, vbo_position_, vbo_texcoord_;
	unsigned ebo_;
	int uniform_size_, uniform_color_;

	std::vector<float> pos_buffer;
	std::vector<unsigned short> texcoord_buffer;
};

}

#endif
