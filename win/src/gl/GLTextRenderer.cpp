#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <cstring>
#include <climits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <win/gl/GLFont.hpp>
#include <win/gl/GLTextRenderer.hpp>

using namespace win::gl;

// font shaders
static const char *vertexshader =
"#version 440 core\n"

"struct Object { vec2 position; uint dims; float index; };\n"

"layout (std140) uniform object_data { Object data[900]; };\n"
"uniform mat4 projection;\n"
"uniform float width;\n"
"uniform float height;\n"

"layout (location = 0) in vec2 vert;\n"
"layout (location = 1) in ivec2 texcoord;\n"
"layout (location = 2) in int draw_id;\n"

"out vec3 ftexcoord;\n"

"void main(){\n"
"int i = draw_id % 900;\n"
"float w = (data[i].dims >> 16) / float(65535);\n"
"float h = (data[i].dims & 65535) / float(65535);\n"
"ftexcoord = vec3(float(texcoord.x) * w, float(texcoord.y) * h, data[i].index);\n"
"mat4 t = mat4(width * w, 0.0, 0.0, 0.0,    0.0, height * h, 0.0, 0.0,    0.0, 0.0, 0.0, 0.0,    data[i].position.x, data[i].position.y, 0.0, 1.0);\n"
"gl_Position = projection * t * vec4(vert.xy, 0.0, 1.0);\n"
"}\n"

, *fragmentshader =
"#version 440 core\n"

"out vec4 pixel;\n"
"in vec3 ftexcoord;\n"

"uniform sampler2DArray tex;\n"
"uniform vec4 color;\n"

"void main(){\n"
"pixel = vec4(1.0, 1.0, 1.0, texture(tex, ftexcoord).r) * color;\n"
"}\n"
;

namespace win
{

GLTextRenderer::GLTextRenderer(const Dimensions<int> &screen_pixel_dimensions, const Area<float> &screen_area, GLenum texture_unit, bool texture_unit_owned)
	: TextRenderer(screen_pixel_dimensions, screen_area)
	, texture_unit(texture_unit)
	, texture_unit_owned(texture_unit_owned)
	, current_font(NULL)
	, current_color(0.0f, 0.0f, 0.0f, 1.0f)
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
	if (uniform_color == -1) win::bug("GLTextRenderer - no uniform color");
	glUniform4f(uniform_color, current_color.red, current_color.green, current_color.blue, current_color.alpha);

	uniform_projection = glGetUniformLocation(program.get(), "projection");
	if (uniform_projection == -1) win::bug("GLTextRenderer - no uniform projection");
	const glm::mat4 projection = glm::ortho(screen_area.left, screen_area.right, screen_area.bottom, screen_area.top);
	glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection));

	if ((uniform_width = glGetUniformLocation(program.get(), "width")) == -1)
		win::bug("no width");
	if ((uniform_height = glGetUniformLocation(program.get(), "height")) == -1)
		win::bug("no height");

	const auto uniform_sampler = glGetUniformLocation(program.get(), "tex");
	if (uniform_sampler == -1) win::bug("no tex");
	glUniform1i(uniform_sampler, texture_unit - GL_TEXTURE0);

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

GLFont GLTextRenderer::create_font(float size, win::Stream data)
{
	return std::move(GLFont(screen_pixel_dimensions, screen_area, size, std::move(data)));
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
	glUseProgram(program.get());
	glBindVertexArray(vao.get());

	for (const auto &str : string_queue)
	{
		const GLFont &font = *(GLFont*)str.font;
		const FontMetric &metric = font.font_metric();

		if (current_font != &font)
		{
			send();
			current_font = &font;

			glUniform1f(uniform_width, metric.max_width);
			glUniform1f(uniform_height, metric.max_height);

			glActiveTexture(texture_unit);
			glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture());
		}
		else if (!texture_unit_owned)
		{
			glActiveTexture(texture_unit);
			glBindTexture(GL_TEXTURE_2D_ARRAY, font.texture());
		}

		if (current_color != str.color)
		{
			send();
			current_color = str.color;

			glUniform4f(uniform_color, str.color.red, str.color.green, str.color.blue, str.color.alpha);
		}

		auto begin = text_queue.begin() + str.text_queue_start;
		auto end = begin + str.text_queue_length;
		for (auto it = begin; it != end; ++it)
		{
			if (object_data_prebuf.size() == object_data_length)
				send();

			const TextRendererCharacter &c = *it;
			const auto &cmetric = font.character_metric(c.c);

			const float xpos = c.xpos + (cmetric.width / 2.0f);
			const float ypos = (c.ypos + (cmetric.height / 2.0f));
			const std::uint16_t width_pct = (cmetric.width / metric.max_width) * std::numeric_limits<std::uint16_t>::max();
			const std::uint16_t height_pct = (cmetric.height / metric.max_height) * std::numeric_limits<std::uint16_t>::max();
			const std::uint32_t dims = (width_pct << 16) | height_pct;
			const float index = c.c - Font::char_low;

			object_data_prebuf.emplace_back();
			ObjectBytes &bytes = object_data_prebuf.back();
			memcpy(bytes.data() + 0, &xpos, sizeof(xpos));
			memcpy(bytes.data() + 4, &ypos, sizeof(ypos));
			memcpy(bytes.data() + 8, &dims, sizeof(dims));
			memcpy(bytes.data() + 12, &index, sizeof(index));
		}
	}

	if (!object_data_prebuf.empty())
		send();

	text_queue.clear();
	string_queue.clear();
}

void GLTextRenderer::send()
{
	const int count = object_data_prebuf.size();

	if (count == 0)
		return;

	auto range = object_data.reserve(count);
	range.write(object_data_prebuf.data(), count);

	glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL, count, range.head());
	object_data.lock(range);

	object_data_prebuf.clear();
}

}

#endif
