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
	glUniform4f = (decltype(glUniform4f))getproc("glUniform4f");
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

const char *win::key_name(const button key)
{
	if(key == button::MOUSE_LEFT)
		return "LeftMouse";
	else if(key == button::MOUSE_RIGHT)
		return "RightMouse";
	else if(key == button::MOUSE_MIDDLE)
		return "MiddleMouse";
	else if(key == button::MOUSE_LEFT)
		return "LeftMouse";
	else if(key == button::MOUSE4)
		return "Mouse4";
	else if(key == button::MOUSE5)
		return "Mouse5";
	else if(key == button::A)
		return "A";
	else if(key == button::B)
		return "B";
	else if(key == button::C)
		return "C";
	else if(key == button::D)
		return "D";
	else if(key == button::E)
		return "E";
	else if(key == button::F)
		return "F";
	else if(key == button::G)
		return "G";
	else if(key == button::H)
		return "H";
	else if(key == button::I)
		return "I";
	else if(key == button::J)
		return "J";
	else if(key == button::K)
		return "K";
	else if(key == button::L)
		return "L";
	else if(key == button::M)
		return "M";
	else if(key == button::N)
		return "N";
	else if(key == button::O)
		return "O";
	else if(key == button::P)
		return "P";
	else if(key == button::Q)
		return "Q";
	else if(key == button::R)
		return "R";
	else if(key == button::S)
		return "S";
	else if(key == button::T)
		return "T";
	else if(key == button::U)
		return "U";
	else if(key == button::V)
		return "V";
	else if(key == button::W)
		return "W";
	else if(key == button::X)
		return "X";
	else if(key == button::Y)
		return "Y";
	else if(key == button::Z)
		return "Z";

	else if(key == button::D0)
		return "0";
	else if(key == button::D1)
		return "1";
	else if(key == button::D2)
		return "2";
	else if(key == button::D3)
		return "3";
	else if(key == button::D4)
		return "4";
	else if(key == button::D5)
		return "5";
	else if(key == button::D6)
		return "6";
	else if(key == button::D7)
		return "7";
	else if(key == button::D8)
		return "8";
	else if(key == button::D9)
		return "9";

	else if(key == button::TILDE)
		return "Tilde";
	else if(key == button::DASH)
		return "Dash";
	else if(key == button::EQUALS)
		return "Equals";
	else if(key == button::LBRACKET)
		return "LeftBracket";
	else if(key == button::RBRACKET)
		return "RightBracket";
	else if(key == button::SEMICOLON)
		return "Semicolon";
	else if(key == button::APOSTROPHE)
		return "Apostrophe";
	else if(key == button::COMMA)
		return "Comma";
	else if(key == button::PERIOD)
		return "Period";
	else if(key == button::SLASH)
		return "Slash";
	else if(key == button::BACKSLASH)
		return "BackSlash";

	else if(key == button::F1)
		return "Function1";
	else if(key == button::F2)
		return "Function2";
	else if(key == button::F3)
		return "Function3";
	else if(key == button::F4)
		return "Function4";
	else if(key == button::F5)
		return "Function5";
	else if(key == button::F6)
		return "Function6";
	else if(key == button::F7)
		return "Function7";
	else if(key == button::F8)
		return "Function8";
	else if(key == button::F9)
		return "Function9";
	else if(key == button::F10)
		return "Function10";
	else if(key == button::F11)
		return "Function11";
	else if(key == button::F12)
		return "Function12";

	else if(key == button::ESC)
		return "Escape";
	else if(key == button::PRINT_SCR)
		return "PrintScreen";
	else if(key == button::PAUSE_BREAK)
		return "PauseBreak";
	else if(key == button::INSERT)
		return "Insert";
	else if(key == button::DELETE)
		return "Delete";
	else if(key == button::HOME)
		return "Home";
	else if(key == button::PAGE_UP)
		return "PageUp";
	else if(key == button::PAGE_DOWN)
		return "PageDown";
	else if(key == button::END)
		return "End";
	else if(key == button::BACKSPACE)
		return "Backspace";
	else if(key == button::RETURN)
		return "Return";
	else if(key == button::ENTER)
		return "Enter";
	else if(key == button::LSHIFT)
		return "LeftShift";
	else if(key == button::RSHIFT)
		return "RightShift";
	else if(key == button::LCTRL)
		return "LeftControl";
	else if(key == button::RCTRL)
		return "RightControl";
	else if(key == button::LALT)
		return "LeftAlt";
	else if(key == button::RALT)
		return "RightAlt";
	else if(key == button::SPACE)
		return "Spacebar";
	else if(key == button::MENU)
		return "Menu";
	else if(key == button::LMETA)
		return "LeftMeta";
	else if(key == button::UP)
		return "UpArrow";
	else if(key == button::LEFT)
		return "LeftArrow";
	else if(key == button::RIGHT)
		return "RightArrow";
	else if(key == button::DOWN)
		return "DownArrow";
	else if(key == button::CAPSLOCK)
		return "CapsLock";
	else if(key == button::TAB)
		return "Tab";

	else if(key == button::NUM_LOCK)
		return "NumLock";
	else if(key == button::NUM_SLASH)
		return "NumSlash";
	else if(key == button::NUM_MULTIPLY)
		return "NumMultiply";
	else if(key == button::NUM_MINUS)
		return "NumMinus";
	else if(key == button::NUM_PLUS)
		return "NumPlus";
	else if(key == button::NUM_DEL)
		return "NumDelete";
	else if(key == button::NUM_0)
		return "Num0";
	else if(key == button::NUM_1)
		return "Num1";
	else if(key == button::NUM_2)
		return "Num2";
	else if(key == button::NUM_3)
		return "Num3";
	else if(key == button::NUM_4)
		return "Num4";
	else if(key == button::NUM_5)
		return "Num5";
	else if(key == button::NUM_6)
		return "Num6";
	else if(key == button::NUM_7)
		return "Num7";
	else if(key == button::NUM_8)
		return "Num8";
	else if(key == button::NUM_9)
		return "Num9";

	return "UndefinedKey";
}
