// most of this code dirty nasty

#ifndef NOGUI

#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS
#include <Commdlg.h>
#endif

#include <win/Display.hpp>
#include <win/gl/GL.hpp>
#include <win/Event.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "atlasizer.hpp"
#include "layoutexporter.hpp"

using namespace win::gl;

class GUIAtlasItem : public AtlasItem
{
public:
	enum class Side { NONE, LEFT, RIGHT, BOTTOM, TOP };

	GUIAtlasItem(int original_index, const std::string &filename, int x, int y)
		: AtlasItem(filename, x, y)
		, original_index(original_index)
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

	Side correct(const std::list<GUIAtlasItem> &items, int padding)
	{
		for (const GUIAtlasItem &item : items)
		{
			if (this == &item)
				continue;

			Side side;
			side = correct(item, padding);

			if (side != Side::NONE)
				return side;
		}

		return Side::NONE;
	}

	Side correct(const AtlasItem &item, int padding)
	{
		if (!colliding(item, padding))
			return Side::NONE;

		const int ldiff = std::abs(x - (item.x + item.width + padding));
		const int rdiff = std::abs((x + width) - (item.x + padding));
		const int tdiff = std::abs((y + height) - (item.y + padding));
		const int bdiff = std::abs(y - (item.y + item.height + padding));

		int smallest = ldiff;
		if (rdiff < smallest) smallest = rdiff;
		if (tdiff < smallest) smallest = tdiff;
		if (bdiff < smallest) smallest = bdiff;

		if (smallest == ldiff)
		{
			x = item.x + item.width + padding;
			return Side::LEFT;
		}
		else if (smallest == rdiff)
		{
			x = item.x - width - padding;
			return Side::RIGHT;
		}
		else if (smallest == tdiff)
		{
			y = item.y - height - padding;
			return Side::TOP;
		}
		else
		{
			y = item.y + item.height + padding;
			return Side::BOTTOM;
		}
	}

	int original_index;
	unsigned texture;
};

struct DragBarrier
{
	DragBarrier() : active(false), location(0) {}
	bool active;
	int location;
};

static const char *helptext =
	"atlasizer: Create a texture atlas. Drag items around, right-click and drag to pan.\n\nI: Import layout\nA: Add item\nS: Save\nE: Export\nDELETE: Delete item\nCTRL (hold): Snap Mode\nSHIFT (hold): Color Mode\n+: Zoom in\n-: Zoom out";

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
	"if(highlight_mode == 1){\n" // colliding
	"	color.r += float(int(gl_FragCoord.x) % 3 == 0);\n"
	"	color.g -= 1.0;\n"
	"	color.b -= 1.0;\n"
	"}\n"
	"else if(highlight_mode == 2){\n" // colliding, but unfocused
	"	color.r += float(int(gl_FragCoord.x) % 3 == 0) / 3.0;\n"
	"	color.g -= 0.5;\n"
	"	color.b -= 0.5;\n"
	"}\n"
	"else if (highlight_mode == 3){\n" // selected, not colliding
	"	color.r += 0.5;\n"
	"	color.g += 0.5;\n"
	"	color.b += 0.5;\n"
	"}"
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
	"uniform bool dirty;\n"
	"out vec4 color;\n"
	"void main(){\n"
	"color = dirty ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}";

static std::string pick_file(bool open, std::string default_file_name, std::string ext_filter)
{
	/*
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
	*/

#if defined WINPLAT_WINDOWS
	std::string filter_description;
	if (ext_filter == "tga")
		filter_description = std::string("TARGA images") + (char)0 + "*.tga" + (char)0 + (char)0;
	else if (ext_filter == "txt")
		filter_description = std::string("Text files") + (char)0 + "*.txt" + (char)0 + (char)0;

	char filebuf[1024] = "";
	if (!open)
		strncpy(filebuf, default_file_name.c_str(), sizeof(filebuf));
	filebuf[sizeof(filebuf) - 1] = 0;

	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter_description.length() > 0 ? (filter_description).c_str() : NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filebuf;
	ofn.nMaxFile = sizeof(filebuf);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = open ? "Open file" : "Save file";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = !open ? ext_filter.c_str() : NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (open)
		ofn.Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	else
		ofn.Flags |= OFN_OVERWRITEPROMPT;

	if (open)
	{
		if (!GetOpenFileNameA(&ofn))
			return "";
	}
	else
	{
		if (!GetSaveFileNameA(&ofn))
			return "";
	}

	filebuf[sizeof(filebuf) - 1] = 0;

	return filebuf;
#elif defined WINPLAT_LINUX
	std::string filename;
	FILE *zenity;

	std::string cmd;
	if (open)
		cmd = "zenity --title=\"Open TGA\" --file-selection --file-filter=\"*." + ext_filter + "\"";
	else
		cmd = "zenity --title=\"Save atlas layout\" --file-selection --save --confirm-overwrite --filename=\"" + (default_file_name.length() == 0 ? "atlas.txt" : default_file_name) + "\"";

	if ((zenity = popen(cmd.c_str(), "r")) == NULL)
	{
		std::cerr << "zenity is required for dialogs" << std::endl;
		return "";
	}

	char file[512];
	fgets(file, 512, zenity);
	if (pclose(zenity) == 0)
		filename = file;
	else
		filename = ""; // this means no file was chosen

	if (filename.length() > 0 && filename.at(filename.size() - 1) == '\n')
		filename = filename.substr(0, filename.size() - 1);
	return filename;
#endif
}

void msg_box(const std::string &msg, bool error)
{
#if defined WINPLAT_WINDOWS
	MessageBox(NULL, msg.c_str(), error ? "Error" : "Info", error ? MB_ICONERROR : 0);
#elif defined WINPLAT_LINUX
	FILE *zenity;

	std::string cmd;
	if (error)
		cmd = "zenity --no-wrap --error --text=\"" + msg + "\"";
	else
		cmd = "zenity --info --no-wrap --text=\"" + msg + "\"";

	if ((zenity = popen(cmd.c_str(), "r")) == NULL)
	{
		std::cerr << "zenity is required for dialogs" << std::endl;
		return;
	}

	pclose(zenity);
#endif
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

static void get_atlas_dims(const std::list<GUIAtlasItem> &items, int padding, int &width, int &height)
{
	width = 0;
	height = 0;

	for (const GUIAtlasItem &item : items)
	{
		if (item.x + item.width > width)
			width = item.x + item.width;
		if (item.y + item.height > height)
			height = item.y + item.height;
	}

	width += padding;
	height += padding;
}

void gui()
{
	win::DisplayOptions dp;
	dp.caption = "Atlasizer";
	dp.gl_major = 3;
	dp.gl_minor = 3;
	dp.width = 1600;
	dp.height = 900;

	win::Display display(dp);

	std::list<GUIAtlasItem> items;
	bool dirty = false; // workspace is modified
	std::string current_save_file; // for save (without save as)
	int selected_index = -1; // which of the items is selected (clicked)
	int selected_xoff = 0, selected_yoff = 0; // help maintain grab point when dragging
	float pan_oldmousex = 0.0f, pan_oldmousey = 0.0f; // help maintain grab point when panning
	float pan_oldcenterx = 0.0f, pan_oldcentery = 0.0f; // help maintain grab point when panning
	float mousex = 0, mousey = 0; // mouse position, in world coordinates
	int mousex_raw = 0, mousey_raw = 0; // mouse position, in screen coordinates
	DragBarrier left_barrier, right_barrier, down_barrier, up_barrier; // help implement snap mode
	bool right_clicking = false;
	bool left_clicking = false;
	bool grabbing = false;
	bool panning = false;
	bool snapmode = false;
	bool solidmode = false;
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
		case win::Button::f1:
			if (press)
				msg_box(helptext, false);
			break;
		case win::Button::mouse_left:
			left_clicking = press;
			break;
		case win::Button::mouse_right:
			right_clicking = press;
			break;
		case win::Button::del:
			if (selected_index >= 0)
			{
				int index = 0;
				for (auto it = items.begin(); it != items.end();)
				{
					if (index == selected_index)
					{
						it = items.erase(it);
						continue;
					}

					++index;
					++it;
				}

				selected_index = -1;
			}
			break;
		case win::Button::esc:
			selected_index = -1;
			break;
		case win::Button::num_plus:
			if (press)
				zoom += zoom_inc;
			break;
		case win::Button::num_minus:
			if (press)
			{
				if (zoom >= 0.1f)
					zoom -= zoom_inc;
			}
			break;
		case win::Button::lctrl:
		case win::Button::rctrl:
			snapmode = press;
			break;
		case win::Button::tab:
			solidmode = press;
			break;
		case win::Button::space:
			if (press)
			{
				int height, width;
				get_atlas_dims(items, padding, width, height);
				int bytes = width * height * 4;
				double mbytes = bytes / 1024.0 / 1024.0;
				char str[50];
				snprintf(str, sizeof(str), "%.2f", mbytes);
				msg_box(std::to_string(items.size()) + " items.\n" + std::to_string(width) + "x" + std::to_string(height) + "\n" + str + " MBs", false);
			}
			break;
		default: break;
		}
	});

	display.register_character_handler([&](int c)
	{
		switch (c)
		{
		case 'Q':
		case 'q':
			quit = true;
			break;
		case 'a':
		case 'A':
			try
			{
				const std::string file = pick_file(true, "", "tga");
				if (file.length() > 0)
				{
					items.emplace_back(items.size(), file, padding, padding);
					dirty = true;
				}
			}
			catch (const std::exception &e)
			{
				msg_box(e.what(), true);
			}
			break;
		case 'e':
			current_save_file = pick_file(false, current_save_file, "txt");
			if (current_save_file.length() == 0)
				break;
		case 's':
			if (current_save_file.length() < 1)
			{
				msg_box("No current save file", true);
				break;
			}
			try
			{
				// sort by original index right before save
				items.sort([](const GUIAtlasItem &a, const GUIAtlasItem &b) { return a.original_index < b.original_index; });

				LayoutExporter exporter(current_save_file, padding);
				for (const GUIAtlasItem &item : items)
				{
					AtlasItemDescriptor aid;
					aid.filename = item.filename;
					aid.x = item.x;
					aid.y = item.y;
					aid.width = item.width;
					aid.height = item.height;

					exporter.add(aid);
				}

				exporter.save();
				selected_index = -1;
				dirty = false;
			}
			catch (std::exception &e)
			{
				msg_box(e.what(), true);
			}
			break;
		case 'i':
		case 'I':
			try
			{
				const std::string importfile = pick_file(true, "", "txt");
				if (importfile.size() > 0)
				{
					items.clear();
					int pad = 0;
					for (const AtlasItemDescriptor &item : LayoutExporter::import(importfile, padding))
						items.emplace_back(items.size(), item.filename, item.x, item.y);
					dirty = false;
					current_save_file = importfile;
				}
			}
			catch (const std::exception &e)
			{
				current_save_file = "";
				items.clear();
				msg_box(e.what(), true);
			}
			break;
		default:
			if (c >= '0' && c <= '9')
			{
				if (padding != c - '0')
					dirty = true;

				padding = c - '0';
			}
			break;
		}

	});

	display.register_window_handler([&quit](win::WindowEvent event)
	{
		if (event == win::WindowEvent::close)
			quit = true;
	});

	// opengl nonsense

	win::load_gl_functions();

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
			int uniform_dirty;
		} guides;
	} renderstate;

	glClearColor(0.05f, 0.05f, 0.05f, 1.0);

	const float aspect_ratio = (float)display.height() / display.width();
	const glm::mat4 projection = glm::ortho(-500.0f, 500.0f, -500.0f * aspect_ratio, 500.0f * aspect_ratio, -1.0f, 1.0f);

	renderstate.atlasitems.program = win::load_gl_shaders(vertexshader_atlasitem, fragmentshader_atlasitem);
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

	renderstate.guides.program = win::load_gl_shaders(vertexshader_guides, fragmentshader_guides);
	glUseProgram(renderstate.guides.program);
	renderstate.guides.uniform_projection = glGetUniformLocation(renderstate.guides.program, "projection");
	renderstate.guides.uniform_view = glGetUniformLocation(renderstate.guides.program, "view");
	renderstate.guides.uniform_dirty = glGetUniformLocation(renderstate.guides.program, "dirty");
	if (renderstate.guides.uniform_projection == -1)
		win::bug("no uniform projection");
	if (renderstate.guides.uniform_view == -1)
		win::bug("no uniform view");
	if (renderstate.guides.uniform_dirty == -1)
		win::bug("no uniform dirty");

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

	while (!quit)
	{
		display.process();

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
				}
			}
		}
		else
			panning = false;

		bool reset_barriers = false; // snapmode bs
		if (grabbing)
		{
			dirty = true;
			GUIAtlasItem *item	= get_item(items, selected_index);
			if (item == NULL) win::bug("null item");

			item->x = mousex - selected_xoff;
			item->y = mousey - selected_yoff;

			// this snap mode crap is terrible lmao
			if (left_barrier.active)
			{
				if (item->x < left_barrier.location)
					item->x = left_barrier.location;
				else if (item->x > left_barrier.location)
					left_barrier.active = false;
			}
			if (right_barrier.active)
			{
				if (item->x + item->width > right_barrier.location)
					item->x = right_barrier.location - item->width;
				else if (item->x + item->width < right_barrier.location)
					right_barrier.active = false;
			}
			if (down_barrier.active)
			{
				if (item->y < down_barrier.location)
					item->y = down_barrier.location;
				else if (item->y > down_barrier.location)
					down_barrier.active = false;
			}
			if (up_barrier.active)
			{
				if (item->y + item->height > up_barrier.location)
					item->y = up_barrier.location - item->height;
				else if (item->y + item->height < up_barrier.location)
					up_barrier.active = false;
			}

			if (!snapmode)
				reset_barriers = true;
			else
			{
				item->correct_bounds(padding);
				const GUIAtlasItem::Side side = item->correct(items, padding);
				if (side == GUIAtlasItem::Side::LEFT)
				{
					left_barrier.active = true;
					left_barrier.location = item->x;
				}
				else if (side == GUIAtlasItem::Side::RIGHT)
				{
					right_barrier.active = true;
					right_barrier.location = item->x + item->width;
				}
				else if (side == GUIAtlasItem::Side::TOP)
				{
					up_barrier.active = true;
					up_barrier.location = item->y + item->height;
				}
				else if (side == GUIAtlasItem::Side::BOTTOM)
				{
					down_barrier.active = true;
					down_barrier.location = item->y;
				}
			}
		}
		else
			reset_barriers = true;

		if (reset_barriers)
		{
			left_barrier.active = false;
			right_barrier.active = false;
			down_barrier.active = false;
			up_barrier.active = false;
		}

		// rendering
		glClear(GL_COLOR_BUFFER_BIT);

		// guides
		glUseProgram(renderstate.guides.program);
		glUniform1i(renderstate.guides.uniform_dirty, dirty ? 1 : 0);
		glBindVertexArray(renderstate.guides.vao);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		// atlas items
		glm::mat4 view = glm::translate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(zoom, zoom, zoom)), glm::vec3(-centerx, -centery, 0.0));

		glUseProgram(renderstate.atlasitems.program);
		glUniformMatrix4fv(renderstate.atlasitems.uniform_view, 1, false, glm::value_ptr(view));

		glUseProgram(renderstate.guides.program);
		glUniformMatrix4fv(renderstate.guides.uniform_view, 1, false, glm::value_ptr(view));

		const std::vector<float> verts = regenverts(items);
		glBindBuffer(GL_ARRAY_BUFFER, renderstate.atlasitems.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);

		glUseProgram(renderstate.atlasitems.program);
		glBindVertexArray(renderstate.atlasitems.vao);
		int index = 0;
		for (const auto &item : items)
		{
			const bool oob = item.oob(padding);
			const bool colliding = item.colliding(items, padding);
			const bool selected = selected_index == index;

			int highlight_mode = 0;
			if (selected)
				highlight_mode = 3;
			if (selected && (colliding || oob))
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

#endif
