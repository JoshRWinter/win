#include <cstring>
#include <cmath>

#include <win/TextRenderer.hpp>

namespace win
{

void TextRenderer::queue(const Font &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered)
{
	const FontMetric &metric = font.font_metric();
	const auto textlen = strlen(text);
	const auto text_queue_start = text_queue.size();

	float xoffset = centered ? (xpos - (line_length(font, text, 0) / 2.0f)) : xpos;
	float yoffset = ypos;

	int real_len = 0;
	char last_char = 0;
	for (int i = 0; i < textlen; ++i)
	{
		if (text[i] == '\n')
		{
			yoffset -= metric.vertical_advance;
			if (centered)
				xoffset = xpos - (line_length(font, text, i + 1) / 2.0f);
			else
				xoffset = xpos;

			last_char = 0;

			continue;
		}
		else if (text[i] == ' ')
		{
			const float kern = last_char == 0 ? 0.0f : find_kern(text[i], font.character_metric(last_char));

			const FontCharacterMetric &char_metric = font.character_metric(text[i]);
			xoffset += char_metric.advance + kern;

			last_char = ' ';

			continue;
		}
		else if (text[i] < ' ' || text[i] > '~')
		{
			fprintf(stderr, "TextRenderer: Non printing ascii character: %c found in text string", text[i]);
			continue;
		}

		const FontCharacterMetric &char_metric = font.character_metric(text[i]);

		const float kern = last_char == 0 ? 0.0f : find_kern(text[i], font.character_metric(last_char));
		xoffset += kern;

		const float x = xoffset;
		const float y = yoffset - (char_metric.height - char_metric.bearing_y);

		text_queue.emplace_back(text[i], x, y);

		xoffset += (char_metric.advance - char_metric.bearing_x);
		last_char = text[i];
		++real_len;
	}

	string_queue.emplace_back(text_queue_start, real_len, color, font);
}

float TextRenderer::find_kern(char c, const FontCharacterMetric &cmetric_last_char)
{
	// search for a kern
	for (const FontKern &fk : cmetric_last_char.kerns)
	{
		if (fk.right_char == c)
			return fk.kern;
	}

	return 0.0f;
}
float TextRenderer::text_width(const Font &font, const char *text)
{
	const auto len = strlen(text);
	float length = 0.0f;
	float max_length = 0.0f;

	char last_char = 0;

	for (int i = 0; i < len; ++i)
	{
		if (text[i] == '\n')
		{
			if (length > max_length)
				max_length = length;

			length = 0.0f;
			last_char = 0;

			continue;
		}

		const FontCharacterMetric &cmetric = font.character_metric(text[i]);

		const float kern = last_char == 0 ? 0.0f : find_kern(text[i], font.character_metric(last_char));
		length += (cmetric.advance - cmetric.bearing_x) + kern;

		last_char = text[i];
	}

	if (length > max_length)
		max_length = length;

	return max_length;
}

float TextRenderer::line_length(const Font &font, const char *text, int start)
{
	const auto len = strlen(text);
	float length = 0.0f;

	char last_char = 0;

	for (int i = start; i < len; ++i)
	{
		if (text[i] == '\n')
			break;

		const FontCharacterMetric &cmetric = font.character_metric(text[i]);

		const float kern = last_char == 0 ? 0.0f : find_kern(text[i], font.character_metric(last_char));
		length += (cmetric.advance - cmetric.bearing_x) + kern;

		last_char = text[i];
	}

	return length;
}

}
