#define _USE_MATH_DEFINES
#include <cmath>

#include <string.h>

#define WIN_STORAGE
#include <win.h>

namespace win
{

Program::Program()
{
	object = UINT_MAX;
}

Program::Program(GLuint program)
	: object(program)
{ }

Program::Program(Program &&rhs)
{
	object = rhs.object;
	rhs.object = UINT_MAX;
}

Program::~Program()
{
	if (object != UINT_MAX)
		glDeleteProgram(object);
}

Program &Program::operator=(Program &&rhs)
{
	if (object != UINT_MAX)
		glDeleteProgram(object);

	object = rhs.object;
	rhs.object = UINT_MAX;

	return *this;
}

GLuint Program::get() const
{
#ifndef NDEBUG
	if (object == UINT_MAX)
		win::bug("Uninitialized program");
#endif

	return object;
}

Vao::Vao()
{
	glGenVertexArrays(1, &object);
}

Vao::Vao(Vao &&rhs)
{
	object = rhs.object;
	rhs.object = UINT_MAX;
}

Vao::~Vao()
{
	if (object != UINT_MAX)
		glDeleteVertexArrays(1, &object);
}

Vao &Vao::operator=(Vao &&rhs)
{
	if (object != UINT_MAX)
		glDeleteVertexArrays(1, &object);

	object = rhs.object;
	rhs.object = UINT_MAX;

	return *this;
}

GLuint Vao::get() const
{
	return object;
}

Vbo::Vbo()
{
	glGenBuffers(1, &object);
}

Vbo::Vbo(Vbo &&rhs)
{
	object = rhs.object;
	rhs.object = UINT_MAX;
}

Vbo::~Vbo()
{
	if (object != UINT_MAX)
		glDeleteBuffers(1, &object);
}

Vbo &Vbo::operator=(Vbo &&rhs)
{
	if (object != UINT_MAX)
		glDeleteBuffers(1, &object);

	object = rhs.object;
	rhs.object = UINT_MAX;

	return *this;
}

GLuint Vbo::get() const
{
	return object;
}

static void *getproc(const char*);
void load_extensions()
{
	glCreateShader = (decltype(glCreateShader))getproc("glCreateShader");
	glShaderSource = (decltype(glShaderSource))getproc("glShaderSource");
	glCompileShader = (decltype(glCompileShader))getproc("glCompileShader");
	glGetShaderiv = (decltype(glGetShaderiv))getproc("glGetShaderiv");
	glGetShaderInfoLog = (decltype(glGetShaderInfoLog))getproc("glGetShaderInfoLog");
	glGetProgramiv = (decltype(glGetProgramiv))getproc("glGetProgramiv");
	glGetProgramInfoLog = (decltype(glGetProgramInfoLog))getproc("glGetProgramInfoLog");
	glAttachShader = (decltype(glAttachShader))getproc("glAttachShader");
	glDetachShader = (decltype(glDetachShader))getproc("glDetachShader");
	glLinkProgram = (decltype(glLinkProgram))getproc("glLinkProgram");
	glDeleteShader = (decltype(glDeleteShader))getproc("glDeleteShader");
	glCreateProgram = (decltype(glCreateProgram))getproc("glCreateProgram");
	glUseProgram = (decltype(glUseProgram))getproc("glUseProgram");
	glDeleteProgram = (decltype(glDeleteProgram))getproc("glDeleteProgram");
	glGenVertexArrays = (decltype(glGenVertexArrays))getproc("glGenVertexArrays");
	glGenBuffers = (decltype(glGenBuffers))getproc("glGenBuffers");
	glBindVertexArray = (decltype(glBindVertexArray))getproc("glBindVertexArray");
	glBindBuffer = (decltype(glBindBuffer))getproc("glBindBuffer");
	glBufferData = (decltype(glBufferData))getproc("glBufferData");
	glVertexAttribPointer = (decltype(glVertexAttribPointer))getproc("glVertexAttribPointer");
	glEnableVertexAttribArray = (decltype(glEnableVertexAttribArray))getproc("glEnableVertexAttribArray");
	glDeleteVertexArrays = (decltype(glDeleteVertexArrays))getproc("glDeleteVertexArrays");
	glDeleteBuffers = (decltype(glDeleteBuffers))getproc("glDeleteBuffers");
	glGetUniformLocation = (decltype(glGetUniformLocation))getproc("glGetUniformLocation");
	glUniformMatrix4fv = (decltype(glUniformMatrix4fv))getproc("glUniformMatrix4fv");
	glVertexAttribDivisor = (decltype(glVertexAttribDivisor))getproc("glVertexAttribDivisor");
	glUniform1f = (decltype(glUniform1f))getproc("glUniform1f");
	glUniform2f = (decltype(glUniform2f))getproc("glUniform2f");
	glUniform4f = (decltype(glUniform4f))getproc("glUniform4f");
	glUniform1i = (decltype(glUniform1i))getproc("glUniform1i");
	glUniform2i = (decltype(glUniform2i))getproc("glUniform2i");
	glDrawElementsInstanced = (decltype(glDrawElementsInstanced))getproc("glDrawElementsInstanced");
	glBufferSubData = (decltype(glBufferSubData))getproc("glBufferSubData");

#if defined WINPLAT_LINUX
	glXSwapIntervalEXT = (decltype(glXSwapIntervalEXT))getproc("glXSwapIntervalEXT");
#elif defined WINPLAT_WINDOWS
	wglSwapIntervalEXT = (decltype(wglSwapIntervalEXT))getproc("wglSwapIntervalEXT");
#endif
}

unsigned load_shaders(const char *vertex_source, int vertex_length, const char *fragment_source, int fragment_length)
{
	const unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
	const unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vshader, 1, &vertex_source, &vertex_length);
	glShaderSource(fshader, 1, &fragment_source, &fragment_length);
	glCompileShader(vshader);
	glCompileShader(fshader);
	int success = 1;
	char buffer[2000] = "";
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if(success == 0)
	{
		glGetShaderInfoLog(vshader, 2000, NULL, buffer);
		win::bug(std::string("vertex shader:\n") + buffer);
	}
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if(success == 0)
	{
		glGetShaderInfoLog(fshader, 2000, NULL, buffer);
		win::bug(std::string("fragment shader:\n") + buffer);
	}
	unsigned program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	GLint linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(!linked)
	{
		glGetProgramInfoLog(program, 2000, NULL, buffer);
		win::bug(std::string("linker:\n") + buffer);
	}
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return program;
}

unsigned load_shaders(const char *vertex, const char *fragment)
{
	return load_shaders(vertex, strlen(vertex), fragment, strlen(fragment));
}

unsigned load_shaders(AssetRollStream vertex, AssetRollStream fragment)
{
	return load_shaders((char*)vertex.read_all().get(), vertex.size(), (char*)fragment.read_all().get(), fragment.size());
}

void init_ortho(float *matrix,float left,float right,float bottom,float top)
{
	matrix[0] = 2.0f / (right - left);
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;
	matrix[4] = 0.0f;
	matrix[5] = 2.0f / (top - bottom);
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;
	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = -2.0f / (1.0f - -1.0f);
	matrix[11] = 0.0f;
	matrix[12] = -((right + left) / (right - left));
	matrix[13] = -((top + bottom)/(top - bottom));
	matrix[14] = -((1.0f + -1.0f) / (1.0f - -1.0f));
	matrix[15] = 1.0f;
}

#if defined WINPLAT_LINUX
static void *getproc(const char *name)
{
	void *address = (void*)glXGetProcAddress((const unsigned char*)name);
	if(address == NULL)
		win::bug(std::string("") + "Could not get extension \"" + name + "\"");

	return address;
}
#elif defined WINPLAT_WINDOWS
static void *getproc(const char *name)
{
	void *address = (void*)wglGetProcAddress(name);
	if(address == NULL)
	{
		MessageBox(NULL, ("This software requires support for OpenGL:" + std::string(name)).c_str(), "Missing Opengl extension", MB_ICONEXCLAMATION);
		std::abort();
	}

	return address;
}
#endif

const char *key_name(const Button key)
{
	switch(key)
	{
	case Button::MOUSE_LEFT: return "LeftMouse";
	case Button::MOUSE_RIGHT: return "RightMouse";
	case Button::MOUSE_MIDDLE: return "MiddleMouse";
	case Button::MOUSE4: return "Mouse4";
	case Button::MOUSE5: return "Mouse5";
	case Button::MOUSE6: return "Mouse6";
	case Button::MOUSE7: return "Mouse7";

	case Button::A: return "A";
	case Button::B: return "B";
	case Button::C: return "C";
	case Button::D: return "D";
	case Button::E: return "E";
	case Button::F: return "F";
	case Button::G: return "G";
	case Button::H: return "H";
	case Button::I: return "I";
	case Button::J: return "J";
	case Button::K: return "K";
	case Button::L: return "L";
	case Button::M: return "M";
	case Button::N: return "N";
	case Button::O: return "O";
	case Button::P: return "P";
	case Button::Q: return "Q";
	case Button::R: return "R";
	case Button::S: return "S";
	case Button::T: return "T";
	case Button::U: return "U";
	case Button::V: return "V";
	case Button::W: return "W";
	case Button::X: return "X";
	case Button::Y: return "Y";
	case Button::Z: return "Z";

	case Button::D0: return "0";
	case Button::D1: return "1";
	case Button::D2: return "2";
	case Button::D3: return "3";
	case Button::D4: return "4";
	case Button::D5: return "5";
	case Button::D6: return "6";
	case Button::D7: return "7";
	case Button::D8: return "8";
	case Button::D9: return "9";

	case Button::BACKTICK: return "BackTick";
	case Button::DASH: return "Dash";
	case Button::EQUALS: return "Equals";
	case Button::LBRACKET: return "LeftBracket";
	case Button::RBRACKET: return "RightBracket";
	case Button::SEMICOLON: return "Semicolon";
	case Button::APOSTROPHE: return "Apostrophe";
	case Button::COMMA: return "Comma";
	case Button::PERIOD: return "Period";
	case Button::SLASH: return "Slash";
	case Button::BACKSLASH: return "BackSlash";

	case Button::F1: return "Function1";
	case Button::F2: return "Function2";
	case Button::F3: return "Function3";
	case Button::F4: return "Function4";
	case Button::F5: return "Function5";
	case Button::F6: return "Function6";
	case Button::F7: return "Function7";
	case Button::F8: return "Function8";
	case Button::F9: return "Function9";
	case Button::F10: return "Function10";
	case Button::F11: return "Function11";
	case Button::F12: return "Function12";

	case Button::ESC: return "Escape";
	case Button::PRINT_SCR: return "PrintScreen";
	case Button::PAUSE_BREAK: return "PauseBreak";
	case Button::INSERT: return "Insert";
	case Button::DELETE: return "Delete";
	case Button::HOME: return "Home";
	case Button::PAGE_UP: return "PageUp";
	case Button::PAGE_DOWN: return "PageDown";
	case Button::END: return "End";
	case Button::BACKSPACE: return "Backspace";
	case Button::RETURN: return "Return";
	case Button::ENTER: return "Enter";
	case Button::LSHIFT: return "LeftShift";
	case Button::RSHIFT: return "RightShift";
	case Button::LCTRL: return "LeftControl";
	case Button::RCTRL: return "RightControl";
	case Button::LALT: return "LeftAlt";
	case Button::RALT: return "RightAlt";
	case Button::SPACE: return "Spacebar";
	case Button::MENU: return "Menu";
	case Button::LMETA: return "LeftMeta";
	case Button::RMETA: return "RightMeta";
	case Button::UP: return "UpArrow";
	case Button::LEFT: return "LeftArrow";
	case Button::RIGHT: return "RightArrow";
	case Button::DOWN: return "DownArrow";
	case Button::CAPSLOCK: return "CapsLock";
	case Button::TAB: return "Tab";

	case Button::NUM_LOCK: return "NumLock";
	case Button::NUM_SLASH: return "NumSlash";
	case Button::NUM_MULTIPLY: return "NumMultiply";
	case Button::NUM_MINUS: return "NumMinus";
	case Button::NUM_PLUS: return "NumPlus";
	case Button::NUM_DEL: return "NumDelete";
	case Button::NUM0: return "Num0";
	case Button::NUM1: return "Num1";
	case Button::NUM2: return "Num2";
	case Button::NUM3: return "Num3";
	case Button::NUM4: return "Num4";
	case Button::NUM5: return "Num5";
	case Button::NUM6: return "Num6";
	case Button::NUM7: return "Num7";
	case Button::NUM8: return "Num8";
	case Button::NUM9: return "Num9";

	case Button::UNDEFINED: return "Unkown";
	}

	return "UndefinedKey";
}

float distance(float x1, float y1, float x2, float y2)
{
	return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

static float normalize(float a)
{
	const float PI2 = 2.0f * M_PI;

	while(a > PI2)
		a -= PI2;

	while(a < 0.0f)
		a += PI2;

	return a;
}

float align(float angle, float target, const float step)
{
	target = normalize(target);
	angle = normalize(angle);

	if(target > angle)
	{
		if(target - angle > M_PI)
		{
			angle -= step;

			if(angle < 0.0f)
			{
				angle += 2.0f * M_PI;

				if(angle < target)
					angle = target;
			}
		}
		else
		{
			angle += step;

			if(angle > target)
				angle = target;
		}
	}
	else
	{
		if(angle - target > M_PI)
		{
			angle += step;

			if(angle > 2.0f * M_PI)
			{
				angle -= 2.0f * M_PI;

				if(angle > target)
					angle = target;
			}
		}
		else
		{
			angle -= step;

			if(angle < target)
				angle = target;
		}
	}

	return angle;
}

float angle_diff(float a, float b)
{
	a = normalize(a);
	b = normalize(b);

	const float diff = fabs(a - b);
	if(diff > M_PI)
		return diff - M_PI;

	return diff;
}

float zerof(float f, float approach)
{
	if(f > 0.0f)
	{
		f -= approach;
		if(f < 0.0f)
			f = 0.0f;
	}
	else if(f < 0.0f)
	{
		f += approach;
		if(f > 0.0f)
			f = 0.0f;
	}

	return f;
}

}
