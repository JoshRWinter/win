#include <win.h>

#include <climits>

#define private public
#include "targa.hpp"
#undef private

static const char *vertexshader_atlasitem =
	"#version 330 core\n"
	"uniform mat4 projection;\n"
	"layout (location = 0) in vec2 pos;\n"
	"layout (location = 1) in vec2 texcoord;\n"
	"out vec2 ftexcoord;\n"
	"void main(){\n"
	"ftexcoord = texcoord;\n"
	"gl_Position = projection * vec4(pos.xy, 0.0, 1.0);\n"
	"}",
	*fragmentshader_atlasitem =
	"#version 330 core\n"
	"out vec4 color;\n"
	"in vec2 ftexcoord;\n"
	"uniform sampler2D tex;\n"
	"void main(){\n"
	"color = /*vec4(1.0, 1.0, 0.0, 1.0);//*/texture(tex, ftexcoord);\n"
	"}";

static const char *vertexshader_guides =
	"#version 330 core\n"
	"uniform mat4 projection;\n"
	"layout (location = 0) in vec2 pos;\n"
	"void main(){\n"
	"gl_Position = projection * vec4(pos.xy, 0.0, 1.0);\n"
	"}",
	*fragmentshader_guides =
	"#version 330 core\n"
	"out vec4 color;\n"
	"void main(){\n"
	"color = vec4(1.0, 0.5, 0.65, 1.0);\n"
	"}";

struct AtlasItem
{
	AtlasItem(std::unique_ptr<unsigned char[]> &&imgdata, int x, int y, int w, int h)
		: imgdata(std::move(imgdata)), x(x), y(y), width(w), height(h)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, this->imgdata.get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	AtlasItem(const AtlasItem&) = delete;
	AtlasItem(AtlasItem&&) = delete;

	void operator=(const AtlasItem&) = delete;
	void operator=(AtlasItem&&) = delete;

	~AtlasItem()
	{
		glDeleteTextures(1, &texture);
	}

	int x;
	int y;
	int width;
	int height;

	std::unique_ptr<unsigned char[]> imgdata;
	unsigned texture;
};

#include <random>
std::mt19937 gen(69420);
static int randomint(int low, int high)
{
	return std::uniform_int_distribution<int>(low, high)(gen);
}
static Targa add_item()
{
	Targa tga("/home/josh/programming/win/tools/atlasizer/test1.tga");
	tga.release();
	tga.w = randomint(200, 450);
	tga.h = randomint(200, 450);
	tga.bytes.reset(new unsigned char[tga.w * tga.h * 4]);

	const int r = randomint(128, 255);
	const int g = randomint(128, 255);
	const int b = randomint(128, 255);

	for (int i = 0; i < tga.w * tga.h * 4; i += 4)
	{
		tga.bytes[i + 0] = r;
		tga.bytes[i + 1] = g;
		tga.bytes[i + 2] = b;
		tga.bytes[i + 3] = 255;
	}

	return std::move(tga);
}

static std::vector<float> regenverts(const std::list<AtlasItem> &items)
{
	std::vector<float> verts;

	for (const AtlasItem &item : items)
	{
		verts.push_back(item.x); verts.push_back(item.y + item.height); verts.push_back(0.0f); verts.push_back(1.0f);
		verts.push_back(item.x); verts.push_back(item.y); verts.push_back(0.0f); verts.push_back(0.0f);
		verts.push_back(item.x + item.width); verts.push_back(item.y); verts.push_back(1.0f); verts.push_back(0.0f);

		verts.push_back(item.x); verts.push_back(item.y + item.height); verts.push_back(0.0f); verts.push_back(1.0f);
		verts.push_back(item.x + item.width); verts.push_back(item.y); verts.push_back(1.0f); verts.push_back(0.0f);
		verts.push_back(item.x + item.width); verts.push_back(item.y + item.height); verts.push_back(1.0f); verts.push_back(1.0f);
	}

	return verts;
}

static int get_index(const std::list<AtlasItem> &items, const AtlasItem &i)
{
	int index = 0;
	for (const AtlasItem &item : items)
	{
		if (&item == &i)
			return index;

		++index;
	}

	return -1;
}

static AtlasItem *get_item(std::list<AtlasItem> &items, int index)
{
	int i = 0;
	for (AtlasItem &item : items)
	{
		if (i == index)
			return &item;

		++i;
	}

	return NULL;
}

static int select_item(std::list<AtlasItem> &items, int x, int y, int &xoff, int &yoff)
{
	const AtlasItem *selected = NULL;
	for (auto it = items.rbegin(); it != items.rend(); ++it)
	{
		if (x > it->x && x < it->x + it->width && y > it->y && y < it->y + it->height)
		{
			xoff = x - it->x;
			yoff = y - it->y;

			selected = &(*it);
			break;
		}
	}

	if (selected == NULL)
		return -1;

	items.sort([selected, &items](const AtlasItem &a, const AtlasItem &b)
	{
		const int a_index = get_index(items, a);
		const int b_index = get_index(items, b);

		if (&a == selected)
			return false;
		if (&b == selected)
			return true;

		return a_index < b_index;
	});

	return items.size() - 1;
}

int main()
{
	win::Display display("Atlasizer", 1600, 900);

	std::list<AtlasItem> items;
	int selected_index = -1;
	int selected_xoff = 0, selected_yoff = 0;
	int pan_anchorx = 0, pan_anchory = 0;

	int mousex = 0, mousey = 0;
	int mousex_raw = 0, mousey_raw = 0;
	bool right_clicking = false;
	bool left_clicking = false;
	bool grabbing = false;
	bool panning = false;

	bool refresh = true;
	float scale = 500.f;
	float centerx = 0.0f, centery = 0.0f;

	const float	scale_inc = 50.0f;

	display.register_mouse_handler([&mousex_raw, &mousey_raw](int x, int y)
	{
		mousex_raw = x;
		mousey_raw = y;
	});

	bool quit = false;
	display.register_button_handler([&](win::Button button, bool press)
	{
		switch (button)
		{
		case win::Button::MOUSE_LEFT:
			left_clicking = press;
			break;
		case win::Button::MOUSE_RIGHT:
			right_clicking = press;
			break;
		case win::Button::ESC:
			quit = true;
			break;
		case win::Button::NUM_PLUS:
			if (press)
			{
				if (scale >= 51.0f)
				{
					scale -= scale_inc;
					refresh = true;
				}
			}
			break;
		case win::Button::NUM_MINUS:
			if (press)
			{
				scale += scale_inc;
				refresh = true;
			}
			break;
		default: break;
		}
	});

	display.register_character_handler([&items, &refresh](int c)
	{
		switch (c)
		{
		case 'o':
		case 'O':
			try
			{
				Targa img = add_item();
				items.emplace_back(std::unique_ptr<unsigned char[]>(img.release()), 0, 0, img.width(), img.height());
				refresh = true;
				break;
			}
			catch (const std::exception &e)
			{
				fprintf(stderr, "%s\n", e.what());
			}
		default: break;
		}
	});

	// opengl nonsense
	win::load_extensions();
	auto program_atlasitem = win::load_shaders(vertexshader_atlasitem, fragmentshader_atlasitem);
	glUseProgram(program_atlasitem);
	auto uniform_projection_atlasitem = glGetUniformLocation(program_atlasitem, "projection");
	if (uniform_projection_atlasitem == -1)
		win::bug("no uniform projection");
	float matrix[16];
	win::init_ortho(matrix, -8.0f, 8.0f, -4.5f, 4.5f);
	glUniformMatrix4fv(uniform_projection_atlasitem, 1, false, matrix);

	unsigned vao_atlasitems, vbo_atlasitems;
	glGenVertexArrays(1, &vao_atlasitems);
	glGenBuffers(1, &vbo_atlasitems);

	glBindVertexArray(vao_atlasitems);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_atlasitems);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	std::vector verts = regenverts(items);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);


	unsigned program_guides = win::load_shaders(vertexshader_guides, fragmentshader_guides);
	glUseProgram(program_guides);
	int uniform_projection_guides = glGetUniformLocation(program_guides, "projection");
	if (uniform_projection_guides == -1)
		win::bug("no uniform projection");

	unsigned vao_guides, vbo_guides;
	glGenVertexArrays(1, &vao_guides);
	glGenBuffers(1, &vbo_guides);
	glBindVertexArray(vao_guides);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_guides);

	const float guide_width = 1.0f;
	const float guide_length = 300.0f;
	float guide_verts[] =
	{
		-guide_width, guide_length,
		-guide_width, 0.0f,
		0.0f, 0.0f,
		-guide_width, guide_length,
		0.0f, 0.0f,
		0.0f, guide_length,

		0.0f, 0.0f,
		0.0f, -guide_width,
		guide_length, -guide_width,
		0.0f, 0.0f,
		guide_length, -guide_width,
		guide_length, 0.0f
	};

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(guide_verts), guide_verts, GL_STATIC_DRAW);

	while (display.process() && !quit)
	{
		// processing
		mousex = ((mousex_raw - (display.width() / 2)) * (scale / (display.width() / 2))) + centerx;
		mousey = (-(mousey_raw - (display.height() / 2)) * (scale / (display.height() / 2) * 0.5625)) + centery;

		if (left_clicking)
		{
			if (!grabbing)
			{
				if ((selected_index = select_item(items, mousex, mousey, selected_xoff, selected_yoff)) != -1)
				{
					grabbing = true;
				}
			}
		}
		else grabbing = false;

		if (right_clicking)
		{
			if (!left_clicking)
			{
				if (!panning)
				{
					panning = true;
					pan_anchorx = mousex_raw;
					pan_anchory = mousey_raw;
				}
				else
				{
					centerx -= (mousex_raw - pan_anchorx);
					centery += mousey_raw - pan_anchory;

					pan_anchorx = mousex_raw;
					pan_anchory = mousey_raw;

					refresh = true;
				}
			}
		}
		else
			panning = false;

		if (grabbing)
		{
			AtlasItem *item	= get_item(items, selected_index);
			if (item == NULL) win::bug("null item");
			item->x = mousex - selected_xoff;
			item->y = mousey - selected_yoff;
			refresh = true;
		}

		// rendering
		if (refresh)
		{
			refresh = false;

			win::init_ortho(matrix, centerx - scale, centerx + scale, centery - (scale * 0.5625f), centery + (scale * 0.5625f));
			glUseProgram(program_atlasitem);
			glUniformMatrix4fv(uniform_projection_atlasitem, 1, false, matrix);
			glUseProgram(program_guides);
			glUniformMatrix4fv(uniform_projection_guides, 1, false, matrix);
			verts = regenverts(items);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_atlasitems);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program_guides);
		glBindVertexArray(vao_guides);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		glUseProgram(program_atlasitem);
		glBindVertexArray(vao_atlasitems);
		int index = 0;
		for (const AtlasItem &item : items)
		{
			glBindTexture(GL_TEXTURE_2D, item.texture);
			glDrawArrays(GL_TRIANGLES, index * 6, 6);
			++index;
		}

		unsigned error;
		if ((error = glGetError()) != 0)
			win::bug(std::to_string(error));

		display.swap();
	}

	glDeleteVertexArrays(1, &vao_atlasitems);
	glDeleteBuffers(1, &vbo_atlasitems);
	glDeleteShader(program_atlasitem);

	glDeleteShader(program_guides);
	glDeleteVertexArrays(1, &vao_guides);
	glDeleteBuffers(1, &vbo_guides);

	return 0;
}
