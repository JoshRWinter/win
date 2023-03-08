#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <win/gl/GL.hpp>
#include <win/Font.hpp>
#include <win/FontRenderer.hpp>

// font shaders
static const char *vertexshader =
"#version 330 core\n"
"layout (location = 0) in vec2 vert;\n"
"layout (location = 1) in vec2 texcoord_offset;\n"
"layout (location = 2) in vec2 pos;\n"
"layout (location = 3) in vec2 texcoord;\n"
"uniform mat4 projection;\n"
"uniform vec2 size;\n"
"out vec2 ftexcoord;\n"
"void main(){\n"
"ftexcoord = vec2(texcoord.x + texcoord_offset.x, texcoord.y - texcoord_offset.y);\n"
"mat4 translate = mat4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, pos.x + (size.x / 2.0), pos.y + (size.y / 2.0), 0.0, 1.0);\n"
"gl_Position = projection * translate * vec4(vert.x * size.x, vert.y * size.y, 0.0, 1.0);\n"
"}\n"
,*fragmentshader =
"#version 330 core\n"
"out vec4 pixel;\n"
"in vec2 ftexcoord;\n"
"uniform sampler2D tex;\n"
"uniform vec4 color;\n"
"void main(){\n"
"pixel = texture(tex, ftexcoord) * color;\n"
"}\n"
;

static constexpr int cols = 16;
static constexpr int rows = 6;

static inline float alignx(int iwidth, float fwidth, float xpos)
{
	return ((((int)((xpos / fwidth) * iwidth)) / (float)iwidth) * fwidth);
}

static inline float aligny(int iheight, float fheight, float ypos)
{
	return ((((int)((ypos / fheight) * iheight)) / (float)iheight) * fheight);
}

namespace win
{

FontRenderer::FontRenderer(const Dimensions<int> &dims, const Area<float> &area)
	: dims(dims)
	, area(area)
{
#ifdef WIN_USE_OPENGL
	// shaders and uniforms
	program = load_gl_shaders(vertexshader, fragmentshader);
	glUseProgram(program);
	uniform_size = glGetUniformLocation(program, "size");
	uniform_color = glGetUniformLocation(program, "color");
	int uniform_projection = glGetUniformLocation(program, "projection");

	glm::mat4 ortho = glm::ortho(area.left, area.right, area.bottom, area.top);
	glUniformMatrix4fv(uniform_projection, 1, false, glm::value_ptr(ortho));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// element indices
	unsigned int indices[] =
	{
		0, 1, 3, 3, 1, 2
	};

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// font vertices
	const float verts[] =
	{
		-0.5f, 0.5f, 0.0f, (1.0f / rows),
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, (1.0f / cols), 0.0f,
		0.5f, 0.5f, (1.0f / cols), (1.0f / rows)
	};

	glGenBuffers(1, &vbo_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	// generic attributes
	glGenBuffers(1, &vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, NULL);

	glGenBuffers(1, &vbo_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, true, 0, NULL);
#endif
}

FontRenderer::~FontRenderer()
{
#ifdef WIN_USE_OPENGL
    glDeleteBuffers(1, &vbo_texcoord);
    glDeleteBuffers(1, &vbo_position);
    glDeleteBuffers(1, &vbo_vertex);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
	glDeleteShader(program);
#endif
}

void FontRenderer::draw(const Font &font, const char *text, float xpos, float ypos, const Color &clr, bool centered)
{
	const int textlen = strlen(text);

	pos_buffer.clear();
	texcoord_buffer.clear();

	// fill vbos
	float xoffset;
	if(centered)
		xoffset = xpos - (line_length(font, text, 0) / 2.0f);
	else
		xoffset = xpos;
	float yoffset = ypos - font.max_bearing_y;
	int charcount = 0;
	for(int i = 0; i < textlen; ++i)
	{
		const int metrics_index = text[i] - ' ';

		if(text[i] == '\n')
		{
			yoffset -= font.vertical;
			if(centered)
				xoffset = xpos - (line_length(font, text, i + 1) / 2.0f);
			else
				xoffset = xpos;
			continue;
		}
		else if(text[i] == ' ')
		{
			xoffset += font.metrics.at(metrics_index).advance - font.metrics.at(metrics_index).bitmap_left;
			continue;
		}

		if(text[i] < ' ' || text[i] > '~')
			win::bug("non printing ascii character: " + std::to_string((int)text[i]) + " found in text string");

#ifdef WIN_USE_OPENGL
		// pos vbo
		pos_buffer.push_back(alignx(dims.width, area.right - area.left, xoffset));
		pos_buffer.push_back(aligny(dims.height, area.top - area.bottom, yoffset + font.metrics.at(metrics_index).bearing_y));

		// texcoord vbo
		const float xnormal = 1.0f / cols;
		const float ynormal = 1.0f / rows;
		unsigned short xcoord = ((float)((text[i] - ' ') % 16) * xnormal) * USHRT_MAX;
		unsigned short ycoord = ((float)((text[i] - ' ') / 16) * ynormal) * USHRT_MAX;
		texcoord_buffer.push_back(xcoord);
		texcoord_buffer.push_back(USHRT_MAX - ycoord);
#endif

		xoffset += font.metrics[metrics_index].advance - font.metrics.at(metrics_index).bitmap_left;

		++charcount;
	}

#ifdef WIN_USE_OPENGL
	glBindVertexArray(vao);
	glUseProgram(program);

	glUniform2f(uniform_size, font.box_width, font.box_height);
	glUniform4f(uniform_color, clr.red, clr.green, clr.blue, clr.alpha);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * charcount * 2, pos_buffer.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned short) * 2 * charcount, texcoord_buffer.data(), GL_DYNAMIC_DRAW);

	glBindTexture(GL_TEXTURE_2D, font.atlas);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, charcount);
#endif
}

// calculate line length, only up to the first newline after <start>
float FontRenderer::line_length(const Font &fnt, const char *text, int start) const
{
	const int len = strlen(text);
	float length = 0.0f;

	for(int i = start; i < len; ++i)
	{
		if(text[i] == '\n')
			break;

		length += fnt.metrics[text[i] - ' '].advance - fnt.metrics[text[i] - ' '].bitmap_left;
	}

	return length;
}

}
