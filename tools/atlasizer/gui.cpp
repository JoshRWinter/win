#include <win.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "atlasizer.hpp"

class GUIAtlasItem : public AtlasItem
{
public:
	GUIAtlasItem(const std::string &filename, int x, int y)
		: AtlasItem(filename, x, y)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, targa.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	~GUIAtlasItem()
	{
		glDeleteTextures(1, &texture);
	}

	GUIAtlasItem(const GUIAtlasItem&) = delete;
	GUIAtlasItem(GUIAtlasItem&&) = delete;
	void operator=(const GUIAtlasItem&) = delete;
	void operator=(GUIAtlasItem&&) = delete;

	bool oob(int padding) const
	{
		return x < padding || y < padding;
	}

	bool colliding(const std::list<GUIAtlasItem> &items, int padding) const
	{
		for (const GUIAtlasItem &item : items)
		{
			if (this == &item)
				continue;

			if (colliding(item, padding))
				return true;
		}

		return false;
	}

	bool colliding(const AtlasItem &item, int padding) const
	{
		return x + width + padding > item.x && x < item.x + item.width + padding && y + height + padding > item.y && y < item.y + item.height + padding;
	}

	void correct_bounds(int padding)
	{
		if (x < padding)
			x = padding;
		if (y < padding)
			y = padding;
	}

	void correct(const std::list<GUIAtlasItem> &items, int padding)
	{
		for (const GUIAtlasItem &item : items)
		{
			if (this == &item)
				continue;

			correct(item, padding);
		}
	}

	void correct(const AtlasItem &item, int padding)
	{
		if (!colliding(item, padding))
			return;

		const int ldiff = std::abs(x - (item.x + item.width + padding));
		const int rdiff = std::abs((x + width) - (item.x + padding));
		const int tdiff = std::abs((y + height) - (item.y + padding));
		const int bdiff = std::abs(y - (item.y + item.height + padding));

		int smallest = ldiff;
		if (rdiff < smallest) smallest = rdiff;
		if (tdiff < smallest) smallest = tdiff;
		if (bdiff < smallest) smallest = bdiff;

		if (smallest == ldiff)
			x = item.x + item.width + padding;
		else if (smallest == rdiff)
			x = item.x - width - padding;
		else if (smallest == tdiff)
			y = item.y - height - padding;
		else if (smallest == bdiff)
			y = item.y + item.height + padding;
	}

	unsigned texture;
};

static const char *vertexshader_atlasitem =
	"#version 330 core\n"
	"uniform mat4 projection;\n"
	"uniform mat4 view;\n"
	"layout (location = 0) in vec2 pos;\n"
	"layout (location = 1) in vec2 texcoord;\n"
	"out vec2 ftexcoord;\n"
	"void main(){\n"
	"ftexcoord = texcoord;\n"
	"gl_Position = projection * view * vec4(pos.xy, 0.0, 1.0);\n"
	"}",
	*fragmentshader_atlasitem =
	"#version 330 core\n"
	"out vec4 color;\n"
	"in vec2 ftexcoord;\n"
	"in vec4 gl_FragCoord;\n"
	"uniform int highlight_mode;\n"
	"uniform int color_mode;\n"
	"uniform sampler2D tex;\n"
	"void main(){\n"
	"if(color_mode == 0)\n"
		"color = texture(tex, ftexcoord);\n"
	"else if (color_mode == 1)\n"
		"color = vec4(1.0, 0.0, 0.0, 1.0)\n;"
	"else\n"
		"color=vec4(0.8, 0.1, 0.1, 1.0);\n"
	"if(highlight_mode == 1){\n"
	"	color.r += float(int(gl_FragCoord.x) % 3 == 0);\n"
	"	color.g -= 1.0;\n"
	"	color.b -= 1.0;\n"
	"}\n"
	"else if(highlight_mode == 2){\n"
	"	color.r += float(int(gl_FragCoord.x) % 3 == 0) / 3.0;\n"
	"	color.g -= 0.5;\n"
	"	color.b -= 0.5;\n"
	"}\n"
	"}";

static const char *vertexshader_guides =
	"#version 330 core\n"
	"uniform mat4 projection;\n"
	"uniform mat4 view;\n"
	"layout (location = 0) in vec2 pos;\n"
	"void main(){\n"
	"gl_Position = projection * view * vec4(pos.xy, 0.0, 1.0);\n"
	"}",
	*fragmentshader_guides =
	"#version 330 core\n"
	"out vec4 color;\n"
	"void main(){\n"
	"color = vec4(1.0, 0.5, 0.65, 1.0);\n"
	"}";

static std::string pick_file()
{
	static std::vector<std::string> files =
	{
		"/home/josh/programming/fishtank/assets_local/mine.tga",
		"/home/josh/programming/fishtank/assets_local/platform_1.tga",
		"/home/josh/programming/fishtank/assets_local/tank.tga",
		"/home/josh/programming/fishtank/assets_local/turret.tga",
		"/home/josh/programming/fishtank/assets_local/platform_3.tga",
		"/home/josh/programming/fishtank/assets_local/artillery.tga"
	};
	static int index = 0;

	return files.at((index++) % files.size());
}

void error_box(const std::string &msg)
{
	fprintf(stderr, "%s\n", msg.c_str());
}

static std::vector<float> regenverts(const std::list<GUIAtlasItem> &items)
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

static int get_index(const std::list<GUIAtlasItem> &items, const GUIAtlasItem &i)
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

static GUIAtlasItem *get_item(std::list<GUIAtlasItem> &items, int index)
{
	int i = 0;
	for (GUIAtlasItem &item : items)
	{
		if (i == index)
			return &item;

		++i;
	}

	return NULL;
}

static int select_item(std::list<GUIAtlasItem> &items, int x, int y, int &xoff, int &yoff)
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

	items.sort([selected, &items](const GUIAtlasItem &a, const GUIAtlasItem &b)
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

static void screen_to_view(const float x, const float y, const int display_width, const int display_height, const int center_x, const int center_y, const float zoom, const float world_width, float &out_x, float &out_y)
{
	glm::vec4 mousepos((((float)x / display_width) * world_width) - (world_width / 2.0f), -(((y / display_height) * world_width) - (world_width	/ 2.0f)) * ((float)display_height / display_width), 0.0f, 1.0f);
	auto view_transform = glm::scale(glm::translate(glm::identity<glm::mat4>(), glm::vec3(center_x, center_y, 0.0f)), glm::vec3(1.0f / zoom, 1.0f / zoom, 1.0f / zoom));
	mousepos = view_transform * mousepos;

	out_x = mousepos.x;
	out_y = mousepos.y;
}

void gui()
{
	win::Display display("Atlasizer", 1600, 900);

	std::list<GUIAtlasItem> items;
	int selected_index = -1; // which of the items is selected (clicked)
	int selected_xoff = 0, selected_yoff = 0; // help maintain grab point when dragging
	float pan_oldmousex = 0.0f, pan_oldmousey = 0.0f; // help maintain grab point when panning
	float pan_oldcenterx = 0.0f, pan_oldcentery = 0.0f; // help maintain grab point when panning
	float mousex = 0, mousey = 0; // mouse position, in world coordinates
	int mousex_raw = 0, mousey_raw = 0; // mouse position, in screen coordinates
	bool right_clicking = false;
	bool left_clicking = false;
	bool grabbing = false;
	bool panning = false;
	bool snapmode = false;
	bool solidmode = false;
	bool refresh = true; // recalculate vertices for atlas items (for rendering on screen)
	int padding = 0; // padding pixels
	float zoom = 1.0; // zoom level
	const float	zoom_inc = 0.1f;
	float centerx = 400, centery = 200; // center of screen, in world coordinates

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
				zoom += zoom_inc;
				refresh = true;
			}
			break;
		case win::Button::NUM_MINUS:
			if (press)
			{
				if (zoom >= 0.1f)
				{
					zoom -= zoom_inc;
					refresh = true;
				}
			}
			break;
		case win::Button::LCTRL:
		case win::Button::RCTRL:
			snapmode = press;
			break;
		case win::Button::RSHIFT:
		case win::Button::LSHIFT:
			solidmode = press;
			break;
		default: break;
		}
	});

	display.register_character_handler([&items, &refresh, &padding](int c)
	{
		switch (c)
		{
		case 'a':
		case 'A':
			try
			{
				const std::string file = pick_file();
				if (file.length() > 0)
				{
					items.emplace_back(pick_file(), padding, padding);
					refresh = true;
				}
				break;
			}
			catch (const std::exception &e)
			{
				error_box(e.what());
			}
		default:
			if (c >= '0' && c <= '9')
			{
				padding = c - '0';
			}
			break;
		}

	});

	// opengl nonsense

	struct
	{
		struct // opengl state for drawing atlas items
		{
			unsigned program;
			unsigned vbo, vao;
			int uniform_projection;
			int uniform_view;
			int uniform_highlight_mode;
			int uniform_color_mode;
		} atlasitems;

		struct // opengl state for drawing the guide lines
		{
			unsigned program;
			unsigned vbo, vao;
			int uniform_projection;
			int uniform_view;
		} guides;
	} renderstate;

	const float aspect_ratio = (float)display.height() / display.width();
	const glm::mat4 projection = glm::ortho(-500.0f, 500.0f, -500.0f * aspect_ratio, 500.0f * aspect_ratio, -1.0f, 1.0f);

	renderstate.atlasitems.program = win::load_shaders(vertexshader_atlasitem, fragmentshader_atlasitem);
	glUseProgram(renderstate.atlasitems.program);
	renderstate.atlasitems.uniform_projection = glGetUniformLocation(renderstate.atlasitems.program, "projection");
	renderstate.atlasitems.uniform_view = glGetUniformLocation(renderstate.atlasitems.program, "view");
	renderstate.atlasitems.uniform_highlight_mode = glGetUniformLocation(renderstate.atlasitems.program, "highlight_mode");
	renderstate.atlasitems.uniform_color_mode = glGetUniformLocation(renderstate.atlasitems.program, "color_mode");
	if (renderstate.atlasitems.uniform_projection == -1)
		win::bug("no uniform projection");
	if (renderstate.atlasitems.uniform_view == -1)
		win::bug("no uniform view");
	if (renderstate.atlasitems.uniform_highlight_mode == -1)
		win::bug("no uniform uniform_highlight_mode");
	if (renderstate.atlasitems.uniform_color_mode == -1)
		win::bug("no uniform uniform_solid");

	glUniformMatrix4fv(renderstate.atlasitems.uniform_projection, 1, false, glm::value_ptr(projection));

	glGenVertexArrays(1, &renderstate.atlasitems.vao);
	glGenBuffers(1, &renderstate.atlasitems.vbo);

	glBindVertexArray(renderstate.atlasitems.vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderstate.atlasitems.vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	renderstate.guides.program = win::load_shaders(vertexshader_guides, fragmentshader_guides);
	glUseProgram(renderstate.guides.program);
	renderstate.guides.uniform_projection = glGetUniformLocation(renderstate.guides.program, "projection");
	renderstate.guides.uniform_view = glGetUniformLocation(renderstate.guides.program, "view");
	if (renderstate.guides.uniform_projection == -1)
		win::bug("no uniform projection");
	if (renderstate.guides.uniform_view == -1)
		win::bug("no uniform view");

	glUniformMatrix4fv(renderstate.guides.uniform_projection, 1, false, glm::value_ptr(projection));

	glGenVertexArrays(1, &renderstate.guides.vao);
	glGenBuffers(1, &renderstate.guides.vbo);
	glBindVertexArray(renderstate.guides.vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderstate.guides.vbo);

	const float guide_width = 5.0f;
	const float guide_length = 400.0f;
	const float guide_start = 0.0f;
	float guide_verts[] =
	{
		-guide_width, guide_length,
		-guide_width, guide_start,
		guide_start, guide_start,
		-guide_width, guide_length,
		guide_start, guide_start,
		guide_start, guide_length,

		guide_start, guide_start,
		guide_start, -guide_width,
		guide_length, -guide_width,
		guide_start, guide_start,
		guide_length, -guide_width,
		guide_length, guide_start
	};

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(guide_verts), guide_verts, GL_STATIC_DRAW);

	while (display.process() && !quit)
	{
		// processing
		screen_to_view(mousex_raw, mousey_raw, display.width(), display.height(), centerx, centery, zoom, 1000.0, mousex, mousey);

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
					pan_oldcenterx = centerx;
					pan_oldcentery = centery;
					pan_oldmousex = mousex;
					pan_oldmousey = mousey;
				}
				else
				{
					float converted_mousex, converted_mousey;
					screen_to_view(mousex_raw, mousey_raw, display.width(), display.height(), pan_oldcenterx, pan_oldcentery, zoom, 1000.0f, converted_mousex, converted_mousey);

					centerx = pan_oldcenterx - (converted_mousex - pan_oldmousex);
					centery = pan_oldcentery - (converted_mousey - pan_oldmousey);

					refresh = true;
				}
			}
		}
		else
			panning = false;

		if (grabbing)
		{
			GUIAtlasItem *item	= get_item(items, selected_index);
			if (item == NULL) win::bug("null item");

			item->x = mousex - selected_xoff;
			item->y = mousey - selected_yoff;

			if (snapmode)
			{
				item->correct(items, padding);
				item->correct_bounds(padding);
			}

			refresh = true;
		}

		// rendering
		if (refresh)
		{
			refresh = false;

			glm::mat4 view = glm::translate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(zoom, zoom, zoom)), glm::vec3(-centerx, -centery, 0.0));

			glUseProgram(renderstate.atlasitems.program);
			glUniformMatrix4fv(renderstate.atlasitems.uniform_view, 1, false, glm::value_ptr(view));

			glUseProgram(renderstate.guides.program);
			glUniformMatrix4fv(renderstate.guides.uniform_view, 1, false, glm::value_ptr(view));

			const std::vector verts = regenverts(items);
			glBindBuffer(GL_ARRAY_BUFFER, renderstate.atlasitems.vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(renderstate.guides.program);
		glBindVertexArray(renderstate.guides.vao);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		glUseProgram(renderstate.atlasitems.program);
		glBindVertexArray(renderstate.atlasitems.vao);
		int index = 0;
		for (const auto &item : items)
		{
			const bool oob = item.oob(padding);
			const bool colliding = item.colliding(items, padding);

			int highlight_mode = 0;
			if (selected_index == index && (colliding || oob))
				highlight_mode = 1;
			else if (colliding || oob)
				highlight_mode = 2;

			int color_mode = 0;
			if (solidmode && selected_index == index)
				color_mode = 1;
			else if (solidmode)
				color_mode = 2;

			glUniform1i(renderstate.atlasitems.uniform_highlight_mode, highlight_mode);
			glUniform1i(renderstate.atlasitems.uniform_color_mode, color_mode);
			glBindTexture(GL_TEXTURE_2D, item.texture);
			glDrawArrays(GL_TRIANGLES, index * 6, 6);
			++index;
		}

#ifndef NDEBUG
		unsigned error;
		if ((error = glGetError()) != 0)
			win::bug(std::to_string(error));
#endif

		display.swap();
	}

	glDeleteVertexArrays(1, &renderstate.atlasitems.vao);
	glDeleteBuffers(1, &renderstate.atlasitems.vbo);
	glDeleteShader(renderstate.atlasitems.program);;

	glDeleteShader(renderstate.guides.program);
	glDeleteVertexArrays(1, &renderstate.guides.vao);
	glDeleteBuffers(1, &renderstate.guides.vbo);
}
