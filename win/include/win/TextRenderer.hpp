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
	WIN_NO_COPY_MOVE(TextRenderer);

public:
	static float text_width(const Font &font, const char *text);
	static float line_length(const Font &font, const char *text, int start = 0);

protected:
	TextRenderer() = default;

	void queue(const Font &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered);

	static float find_kern(char c, const FontCharacterMetric &cmetric_last_char);

	std::vector<TextRendererCharacter> text_queue;
	std::vector<TextRendererString> string_queue;
};

}
