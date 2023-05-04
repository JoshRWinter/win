#pragma once

#include <vector>

#include <win/Font.hpp>

namespace win
{

struct TextRendererCharacter
{
	TextRendererCharacter(char c, float xpos, float ypos)
		: c(c), xpos(xpos), ypos(ypos) {}

	char c;
	float xpos;
	float ypos;
};

struct TextRendererString
{
	TextRendererString(int text_queue_start, int text_queue_length, const Color<float> &color, const Font &font)
		: text_queue_start(text_queue_start)
		, text_queue_length(text_queue_length)
		, color(color)
		, font(&font)
	{}

	int text_queue_start;
	int text_queue_length;
	Color<float> color;
	const Font *font;
};

class TextRenderer
{
public:
	const Dimensions<int> &dimensions() { return screen_pixel_dimensions; }
	const Area<float> &area() { return screen_area; }

	static float text_width(const Font &font, const char *text);
	static float line_length(const Font &font, const char *text, int start = 0);

protected:
	TextRenderer(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area);

	void queue(const Font &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered);

	static float find_kern(char c, const FontCharacterMetric &cmetric_last_char);
	static float align(int pixel_scale, float scale, float f);

	Dimensions<int> screen_pixel_dimensions;
	Area<float> screen_area;

	std::vector<TextRendererCharacter> text_queue;
	std::vector<TextRendererString> string_queue;
};

}
