#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <win/gl/GLFont.hpp>
#include <win/gl/GLTextRenderer.hpp>

using namespace win::gl;

// font shaders
static const char *vertexshader =
"#version 440 core\n"

"struct Object { vec2 position; vec2 dims; vec2 tc; float index; float pad1; };\n"

"layout (std140) uniform object_data { Object data[60]; };\n"
"uniform mat4 projection;\n"

"layout (location = 0) in vec2 vert;\n"
"layout (location = 1) in ivec2 texcoord;\n"
"layout (location = 2) in int draw_id;\n"

"out vec3 ftexcoord;\n"

"void main(){\n"
"int i = draw_id % 60;\n"
"ftexcoord = vec3(float(texcoord.x) * data[i].tc.x, float(texcoord.y) * data[i].tc.y, data[i].index);\n"
"mat4 t = mat4(data[i].dims.x, 0.0, 0.0, 0.0,    0.0, data[i].dims.y, 0.0, 0.0,    0.0, 0.0, 0.0, 0.0,    data[i].position.x, data[i].position.y, 0.0, 1.0);\n"
"gl_Position = projection * t * vec4(vert.xy, 0.0, 1.0);\n"
"}\n"

, *fragmentshader =
"#version 440 core\n"

"out vec4 pixel;\n"
"in vec3 ftexcoord;\n"

"uniform sampler2DArray tex;\n"
"uniform vec4 color;\n"
"uniform bool blink;\n"

"void main(){\n"
"if (blink) pixel = vec4(1.0, 1.0, 1.0, 1.0);\n"
"else pixel = vec4(1.0, 1.0, 1.0, texture(tex, ftexcoord).r) * color;\n"
"}\n"
;

namespace win
{

GLTextRenderer::GLTextRenderer(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area)
	: TextRenderer(screen_pixel_dimensions, screen_area)
{
	const float verts[] =
	{
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f
	};

	const std::uint8_t texcoords[] =
	{
		0, 1,
		0, 0,
		1, 0,
		1, 1
	};

	const std::uint8_t indices[] =
	{
		0, 1, 2, 0, 2, 3
	};

	static_assert((uniform_object_data_length * 2 - 2) <= 32767, "we gon' need a bigger boat here cap'n");
	std::uint16_t draw_ids[(uniform_object_data_length * 2) - 2];
	for (int i = 0; i < (uniform_object_data_length * 2) - 2; ++i)
		draw_ids[i] = i;

	// shaders and uniforms
	program = std::move(GLProgram(load_gl_shaders(vertexshader, fragmentshader)));
	glUseProgram(program.get());

	uniform_color = glGetUniformLocation(program.get(), "color");
	if (uniform_color == -1) win::bug("GLFontRenderer - no uniform color");

	uniform_projection = glGetUniformLocation(program.get(), "projection");
	if (uniform_projection == -1) win::bug("GLFontRenderer - no uniform projection");
	const glm::mat4 projection = glm::ortho(screen_area.left, screen_area.right, screen_area.bottom, screen_area.top);
	glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection));

	uniform_blink = glGetUniformLocation(program.get(), "blink");
	if (uniform_blink == -1) win::bug("no blink");

	glBindVertexArray(vao.get());

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex.get());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	// texture coords
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord.get());
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 2, GL_UNSIGNED_BYTE, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	// draw ids
	glBindBuffer(GL_ARRAY_BUFFER, vbo_drawids.get());
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, 0, NULL);
	glVertexAttribDivisor(2, 1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(draw_ids), draw_ids, GL_STATIC_DRAW);

	// element indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// object data
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_object_data.get());
	const auto object_data_block_index = glGetUniformBlockIndex(program.get(), "object_data");
	if (object_data_block_index == GL_INVALID_INDEX) win::bug("No object data uniform block index");
	glUniformBlockBinding(program.get(), object_data_block_index, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_object_data.get());
	glBufferStorage(GL_UNIFORM_BUFFER, sizeof(ObjectBytes) * object_data_length * object_data_multiplier, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	void *instances_mem = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(ObjectBytes) * object_data_length * object_data_multiplier, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	object_data = std::move(GLMappedRingBuffer<ObjectBytes>(instances_mem, object_data_length * object_data_multiplier));
}

void GLTextRenderer::draw(const GLFont &font, const char *text, float xpos, float ypos, bool centered)
{
	draw(font, text, xpos, ypos, Color<float>(), centered);
}

void GLTextRenderer::draw(const GLFont &font, const char *text, float xpos, float ypos, const Color<float> &color, bool centered)
{
	queue(font, text, xpos, ypos, color, centered);
}

void GLTextRenderer::flush()
{
	static int frame = 0;
	++frame;
	const bool blink = frame % 2 == 0;
	glUseProgram(program.get());
	glBindVertexArray(vao.get());

	for (const auto &str : string_queue)
	{
		const GLFont &font = *(GLFont *)str.font;
		const FontMetric &metric = font.font_metric();
		int text_queue_index = str.text_queue_start;

		while (text_queue_index < str.text_queue_start + str.text_queue_length)
		{
			const int remaining = str.text_queue_length - (text_queue_index - str.text_queue_start);
			const int take = std::min(object_data_length, remaining);

			auto range = object_data.reserve(take);
			for (int range_index = 0; range_index < take; ++range_index)
			{
				const auto &c = text_queue.at(text_queue_index);
				const auto &cmetric = font.character_metric(c.c);

				const float xpos = c.xpos + (cmetric.width / 2.0f);
				const float ypos = (c.ypos + (cmetric.height / 2.0f));
				const float width = cmetric.width;
				const float height = cmetric.height;
				const float tc_s = cmetric.width_pixels / (float)metric.max_width_pixels;
				const float tc_t = cmetric.height_pixels / (float)metric.max_height_pixels;
				const float index = c.c - Font::char_low;

				ObjectBytes bytes;
				memcpy(bytes.data() + 0, &xpos, sizeof(xpos));
				memcpy(bytes.data() + 4, &ypos, sizeof(ypos));
				memcpy(bytes.data() + 8, &width, sizeof(width));
				memcpy(bytes.data() + 12, &height, sizeof(height));
				memcpy(bytes.data() + 16, &tc_s, sizeof(tc_s));
				memcpy(bytes.data() + 20, &tc_t, sizeof(tc_t));
				memcpy(bytes.data() + 24, &index, sizeof(index));

				range[range_index] = bytes;
				++text_queue_index;
			}

			glUniform4f(uniform_color, str.color.red, str.color.green, str.color.blue, str.color.alpha);
			glUniform1i(uniform_blink, blink ? 0 : 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture());
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL, take, range.head());
			object_data.lock(range);
		}
	}

	text_queue.clear();
	string_queue.clear();
}

}

#endif
