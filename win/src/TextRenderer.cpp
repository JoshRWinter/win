#include <cstring>
#include <cmath>

#include <win/TextRenderer.hpp>

namespace win
{

TextRenderer::TextRenderer(const win::Dimensions<int> &screen_pixel_dimensions, const win::Area<float> &screen_area)
	: screen_pixel_dimensions(screen_pixel_dimensions)
	, screen_area(screen_area)
{}

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
			float kern = 0.0f;
			// search for a kern
			if (last_char != 0)
			{
				const FontCharacterMetric &last_char_metric = font.character_metric(last_char);

				for (const FontKern &fk : last_char_metric.kerns)
				{
					if (fk.right_char == text[i])
					{
						kern = fk.kern;
						break;
					}
				}
			}

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

		float kern = 0.0f;
		// search for a kern
		if (last_char != 0)
		{
			const FontCharacterMetric &last_char_metric = font.character_metric(last_char);

			for (const FontKern &fk : last_char_metric.kerns)
			{
				if (fk.right_char == text[i])
				{
					kern = fk.kern;
					break;
				}
			}
		}

		xoffset += kern;

		const float x = align(screen_pixel_dimensions.width, screen_area.right - screen_area.left, xoffset);
		const float y = align(screen_pixel_dimensions.height, screen_area.top - screen_area.bottom, yoffset - (char_metric.height - char_metric.bearing_y));

		text_queue.emplace_back(text[i], x, y);

		xoffset += (char_metric.advance - char_metric.bearing_x);
		last_char = text[i];
		++real_len;
	}

	string_queue.emplace_back(text_queue_start, real_len, color, font);
}

// calculate line length, only up to the first newline after <start>
float TextRenderer::line_length(const Font &font, const char *text, int start)
{
	const auto len = strlen(text);
	float length = 0.0f;

	for (int i = start; i < len; ++i)
	{
		if (text[i] == '\n')
			break;

		length += font.character_metric(text[i]).advance;
	}

	return length;
}

float TextRenderer::align(int pixel_scale, float scale, float f)
{
	return (((int)std::roundf((f / scale) * pixel_scale) / (float)pixel_scale) * scale);
}

}
