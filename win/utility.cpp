#define WIN_STORAGE
#include "win.h"

static void *getproc(const char*);

void win::load_extensions()
{
	glCreateShader = (decltype(glCreateShader))getproc("glCreateShader");
	glShaderSource = (decltype(glShaderSource))getproc("glShaderSource");
	glCompileShader = (decltype(glCompileShader))getproc("glCompileShader");
	glGetShaderiv = (decltype(glGetShaderiv))getproc("glGetShaderiv");
	glGetShaderInfoLog = (decltype(glGetShaderInfoLog))getproc("glGetShaderInfoLog");
	glAttachShader = (decltype(glAttachShader))getproc("glAttachShader");
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
	glDrawElementsInstanced = (decltype(glDrawElementsInstanced))getproc("glDrawElementsInstanced");
	glBufferSubData = (decltype(glBufferSubData))getproc("glBufferSubData");

#if defined WINPLAT_LINUX
	glXSwapIntervalEXT = (decltype(glXSwapIntervalEXT))getproc("glXSwapIntervalEXT");
#endif
}

unsigned win::load_shaders(const char *vertex_source, const char *fragment_source)
{
	const unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
	const unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vshader, 1, &vertex_source, NULL);
	glShaderSource(fshader, 1, &fragment_source, NULL);
	glCompileShader(vshader);
	glCompileShader(fshader);
	int success = 0;
	char buffer[2000];
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if(success == 0)
	{
		glGetShaderInfoLog(vshader, 2000, NULL, buffer);
		throw exception(std::string("vertex shader:\n") + buffer);
	}
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if(success == 0)
	{
		glGetShaderInfoLog(fshader, 2000, NULL, buffer);
		throw exception(std::string("fragment shader:\n") + buffer);
	}
	unsigned program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return program;
}

void win::init_ortho(float *matrix,float left,float right,float bottom,float top)
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
void *getproc(const char *name)
{
	void *address = (void*)glXGetProcAddress((const unsigned char*)name);
	if(name == NULL)
		throw win::exception(std::string("") + "Could not get extension \"" + name + "\"");

	return address;
}
#else
#error "unsupported platform"
#endif

const char *win::key_name(const pkey key)
{
	if(key == pkey::A)
		return "A";
	else if(key == pkey::B)
		return "B";
	else if(key == pkey::C)
		return "C";
	else if(key == pkey::D)
		return "D";
	else if(key == pkey::E)
		return "E";
	else if(key == pkey::F)
		return "F";
	else if(key == pkey::G)
		return "G";
	else if(key == pkey::H)
		return "H";
	else if(key == pkey::I)
		return "I";
	else if(key == pkey::J)
		return "J";
	else if(key == pkey::K)
		return "K";
	else if(key == pkey::L)
		return "L";
	else if(key == pkey::M)
		return "M";
	else if(key == pkey::N)
		return "N";
	else if(key == pkey::O)
		return "O";
	else if(key == pkey::P)
		return "P";
	else if(key == pkey::Q)
		return "Q";
	else if(key == pkey::R)
		return "R";
	else if(key == pkey::S)
		return "S";
	else if(key == pkey::T)
		return "T";
	else if(key == pkey::U)
		return "U";
	else if(key == pkey::V)
		return "V";
	else if(key == pkey::W)
		return "W";
	else if(key == pkey::X)
		return "X";
	else if(key == pkey::Y)
		return "Y";
	else if(key == pkey::Z)
		return "Z";

	else if(key == pkey::D0)
		return "0";
	else if(key == pkey::D1)
		return "1";
	else if(key == pkey::D2)
		return "2";
	else if(key == pkey::D3)
		return "3";
	else if(key == pkey::D4)
		return "4";
	else if(key == pkey::D5)
		return "5";
	else if(key == pkey::D6)
		return "6";
	else if(key == pkey::D7)
		return "7";
	else if(key == pkey::D8)
		return "8";
	else if(key == pkey::D9)
		return "9";

	else if(key == pkey::TILDE)
		return "Tilde";
	else if(key == pkey::DASH)
		return "Dash";
	else if(key == pkey::EQUALS)
		return "Equals";
	else if(key == pkey::LBRACKET)
		return "LeftBracket";
	else if(key == pkey::RBRACKET)
		return "RightBracket";
	else if(key == pkey::SEMICOLON)
		return "Semicolon";
	else if(key == pkey::APOSTROPHE)
		return "Apostrophe";
	else if(key == pkey::COMMA)
		return "Comma";
	else if(key == pkey::PERIOD)
		return "Period";
	else if(key == pkey::SLASH)
		return "Slash";
	else if(key == pkey::BACKSLASH)
		return "BackSlash";

	else if(key == pkey::F1)
		return "Function1";
	else if(key == pkey::F2)
		return "Function2";
	else if(key == pkey::F3)
		return "Function3";
	else if(key == pkey::F4)
		return "Function4";
	else if(key == pkey::F5)
		return "Function5";
	else if(key == pkey::F6)
		return "Function6";
	else if(key == pkey::F7)
		return "Function7";
	else if(key == pkey::F8)
		return "Function8";
	else if(key == pkey::F9)
		return "Function9";
	else if(key == pkey::F10)
		return "Function10";
	else if(key == pkey::F11)
		return "Function11";
	else if(key == pkey::F12)
		return "Function12";

	else if(key == pkey::ESC)
		return "Escape";
	else if(key == pkey::PRINT_SCR)
		return "PrintScreen";
	else if(key == pkey::PAUSE_BREAK)
		return "PauseBreak";
	else if(key == pkey::INSERT)
		return "Insert";
	else if(key == pkey::DELETE)
		return "Delete";
	else if(key == pkey::HOME)
		return "Home";
	else if(key == pkey::PAGE_UP)
		return "PageUp";
	else if(key == pkey::PAGE_DOWN)
		return "PageDown";
	else if(key == pkey::END)
		return "End";
	else if(key == pkey::BACKSPACE)
		return "Backspace";
	else if(key == pkey::RETURN)
		return "Return";
	else if(key == pkey::ENTER)
		return "Enter";
	else if(key == pkey::LSHIFT)
		return "LeftShift";
	else if(key == pkey::RSHIFT)
		return "RightShift";
	else if(key == pkey::LCTRL)
		return "LeftControl";
	else if(key == pkey::RCTRL)
		return "RightControl";
	else if(key == pkey::LALT)
		return "LeftAlt";
	else if(key == pkey::RALT)
		return "RightAlt";
	else if(key == pkey::SPACE)
		return "Spacebar";
	else if(key == pkey::MENU)
		return "Menu";
	else if(key == pkey::LMETA)
		return "LeftMeta";
	else if(key == pkey::UP)
		return "UpArrow";
	else if(key == pkey::LEFT)
		return "LeftArrow";
	else if(key == pkey::RIGHT)
		return "RightArrow";
	else if(key == pkey::DOWN)
		return "DownArrow";
	else if(key == pkey::CAPSLOCK)
		return "CapsLock";
	else if(key == pkey::TAB)
		return "Tab";

	else if(key == pkey::NUM_LOCK)
		return "NumLock";
	else if(key == pkey::NUM_SLASH)
		return "NumSlash";
	else if(key == pkey::NUM_MULTIPLY)
		return "NumMultiply";
	else if(key == pkey::NUM_MINUS)
		return "NumMinus";
	else if(key == pkey::NUM_PLUS)
		return "NumPlus";
	else if(key == pkey::NUM_DEL)
		return "NumDelete";
	else if(key == pkey::NUM_0)
		return "Num0";
	else if(key == pkey::NUM_1)
		return "Num1";
	else if(key == pkey::NUM_2)
		return "Num2";
	else if(key == pkey::NUM_3)
		return "Num3";
	else if(key == pkey::NUM_4)
		return "Num4";
	else if(key == pkey::NUM_5)
		return "Num5";
	else if(key == pkey::NUM_6)
		return "Num6";
	else if(key == pkey::NUM_7)
		return "Num7";
	else if(key == pkey::NUM_8)
		return "Num8";
	else if(key == pkey::NUM_9)
		return "Num9";

	throw exception("key name not recognized");
}
