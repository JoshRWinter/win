#include <cmath>

#include <string.h>

#define WIN_STORAGE
#include <win.h>

win::data::data() noexcept
	: data_(NULL)
	, size_(0)
	, stream_position_(0)
{
}

win::data::data(unsigned char *d, unsigned long long size) noexcept
	: data_(d)
	, size_(size)
	, stream_position_(0)
{
}

win::data::data(data &&rhs) noexcept
	: data_(rhs.data_)
	, size_(rhs.size_)
	, stream_position_(rhs.stream_position_)
{
	rhs.data_ = NULL;
}

win::data::~data()
{
	finalize();
}

win::data &win::data::operator=(data &&rhs) noexcept
{
	finalize();

	data_ = rhs.data_;
	size_ = rhs.size_;
	stream_position_ = rhs.stream_position_;

	rhs.data_ = NULL;

	return *this;
}

bool win::data::operator!() const noexcept
{
	return data_ == NULL;
}

const unsigned char *win::data::get() const noexcept
{
	if(data_ == NULL)
		bug("Null filedata");

	return data_;
}

unsigned char *win::data::release() noexcept
{
	if(data_ == NULL)
		bug("Null filedata");

	unsigned char *d = data_;
	data_ = NULL;
	size_ = 0;
	stream_position_ = 0;

	return d;
}

unsigned long long win::data::size() const noexcept
{
	return size_;
}

unsigned long long win::data::read(void *userbuffer, size_t amount) noexcept
{
	if(data_ == NULL)
		bug("Null filedata");

	const long long to_read = std::min((long long)amount, (long long)(size_ - stream_position_));
	if(to_read < 0)
		bug("Negative read amount");

	memcpy(userbuffer, data_ + stream_position_, to_read);
	stream_position_ += to_read;

	return to_read;
}

void win::data::seek(size_t position) noexcept
{
	stream_position_ = position;

	if(stream_position_ > size_)
		stream_position_ = size_;
}

void win::data::finalize()
{
	delete[] data_;
	data_ = NULL;
}

win::data_list::data_list(roll *parent)
	: parent_(parent)
{
}

void win::data_list::add(const std::string &name)
{
	filenames_.emplace_back(name);
}

win::data win::data_list::get(int index) const
{
	return parent_->operator[](filenames_.at(index).c_str());
}

const std::string &win::data_list::name(int index) const
{
	return filenames_.at(index);
}

int win::data_list::count() const
{
	return filenames_.size();
}

// shader program wrapper
win::program::program()
{
	program_ = (GLuint)-1;
}

win::program::program(GLuint prog)
{
	program_ = prog;
}

win::program::program(program &&rhs)
{
	program_ = rhs.program_;
	rhs.program_ = (GLuint)-1;
}

win::program::~program()
{
	finalize();
}

win::program &win::program::operator=(program &&rhs)
{
	finalize();

	program_ = rhs.program_;
	rhs.program_ = (GLuint)-1;

	return *this;
}

win::program::operator GLuint()
{
#ifndef NDEBUG
	if(program_ == (GLuint)-1)
		win::bug("INVALID SHADER PROGRAM");
#endif

	return program_;
}

void win::program::finalize()
{
	if(program_ != (GLuint)-1)
	{
		glDeleteProgram(program_);
		program_ = (GLuint)-1;
	}
}

// vao wrappper
win::vao::vao()
{
	glGenVertexArrays(1, &vao_);
}

win::vao::vao(vao &&rhs)
{
	vao_ = rhs.vao_;
	rhs.vao_ = (GLuint)-1;
}

win::vao::~vao()
{
	finalize();
}

win::vao &win::vao::operator=(vao &&rhs)
{
	finalize();

	vao_ = rhs.vao_;
	rhs.vao_ = (GLuint)-1;

	return *this;
}

win::vao::operator GLuint()
{
	return vao_;
}

void win::vao::finalize()
{
	if(vao_ != (GLuint)-1)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = (GLuint)-1;
	}
}

// vbo wrapper
win::vbo::vbo()
{
	glGenBuffers(1, &vbo_);
}

win::vbo::vbo(vbo &&rhs)
{
	vbo_ = rhs.vbo_;
	rhs.vbo_ = (GLuint)-1;
}

win::vbo::~vbo()
{
	finalize();
}

win::vbo &win::vbo::operator=(vbo &&rhs)
{
	finalize();

	vbo_ = rhs.vbo_;
	rhs.vbo_ = (GLuint)-1;

	return *this;
}

win::vbo::operator GLuint()
{
	return vbo_;
}

void win::vbo::finalize()
{
	if(vbo_ != (GLuint)-1)
	{
		glDeleteBuffers(1, &vbo_);
		vbo_ = (GLuint)-1;
	}
}

// ebo wrapper
win::ebo::ebo()
{
	glGenBuffers(1, &ebo_);
}

win::ebo::ebo(ebo &&rhs)
{
	ebo_ = rhs.ebo_;
	rhs.ebo_ = (GLuint)-1;
}

win::ebo::~ebo()
{
	finalize();
}

win::ebo &win::ebo::operator=(ebo &&rhs)
{
	finalize();

	ebo_ = rhs.ebo_;
	rhs.ebo_ = (GLuint)-1;

	return *this;
}

win::ebo::operator GLuint()
{
#ifndef NDEBUG
	if(ebo_ == (GLuint)-1)
		win::bug("INVALID ELEMENT BUFFER OBJECT");
#endif

	return ebo_;
}

void win::ebo::finalize()
{
	if(ebo_ != (GLuint)-1)
	{
		glDeleteBuffers(1, &ebo_);
		ebo_ = (GLuint)-1;
	}
}

static void *getproc(const char*);
void win::load_extensions()
{
	glCreateShader = (decltype(glCreateShader))getproc("glCreateShader");
	glShaderSource = (decltype(glShaderSource))getproc("glShaderSource");
	glCompileShader = (decltype(glCompileShader))getproc("glCompileShader");
	glGetShaderiv = (decltype(glGetShaderiv))getproc("glGetShaderiv");
	glGetShaderInfoLog = (decltype(glGetShaderInfoLog))getproc("glGetShaderInfoLog");
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

unsigned win::load_shaders(const char *vertex_source, int vertex_length, const char *fragment_source, int fragment_length)
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
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return program;
}

unsigned win::load_shaders(const char *vertex, const char *fragment)
{
	return load_shaders(vertex, strlen(vertex), fragment, strlen(fragment));
}

unsigned win::load_shaders(const data &vertex, const data &fragment)
{
	return load_shaders((char*)vertex.get(), vertex.size(), (char*)fragment.get(), fragment.size());
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
	if(address == NULL)
		throw win::exception(std::string("") + "Could not get extension \"" + name + "\"");

	return address;
}
#elif defined WINPLAT_WINDOWS
void *getproc(const char *name)
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

const char *win::key_name(const button key)
{
	switch(key)
	{
		case button::MOUSE_LEFT: return "LeftMouse";
		case button::MOUSE_RIGHT: return "RightMouse";
		case button::MOUSE_MIDDLE: return "MiddleMouse";
		case button::MOUSE4: return "Mouse4";
		case button::MOUSE5: return "Mouse5";
		case button::MOUSE6: return "Mouse6";
		case button::MOUSE7: return "Mouse7";

		case button::A: return "A";
		case button::B: return "B";
		case button::C: return "C";
		case button::D: return "D";
		case button::E: return "E";
		case button::F: return "F";
		case button::G: return "G";
		case button::H: return "H";
		case button::I: return "I";
		case button::J: return "J";
		case button::K: return "K";
		case button::L: return "L";
		case button::M: return "M";
		case button::N: return "N";
		case button::O: return "O";
		case button::P: return "P";
		case button::Q: return "Q";
		case button::R: return "R";
		case button::S: return "S";
		case button::T: return "T";
		case button::U: return "U";
		case button::V: return "V";
		case button::W: return "W";
		case button::X: return "X";
		case button::Y: return "Y";
		case button::Z: return "Z";

		case button::D0: return "0";
		case button::D1: return "1";
		case button::D2: return "2";
		case button::D3: return "3";
		case button::D4: return "4";
		case button::D5: return "5";
		case button::D6: return "6";
		case button::D7: return "7";
		case button::D8: return "8";
		case button::D9: return "9";

		case button::BACKTICK: return "BackTick";
		case button::DASH: return "Dash";
		case button::EQUALS: return "Equals";
		case button::LBRACKET: return "LeftBracket";
		case button::RBRACKET: return "RightBracket";
		case button::SEMICOLON: return "Semicolon";
		case button::APOSTROPHE: return "Apostrophe";
		case button::COMMA: return "Comma";
		case button::PERIOD: return "Period";
		case button::SLASH: return "Slash";
		case button::BACKSLASH: return "BackSlash";

		case button::F1: return "Function1";
		case button::F2: return "Function2";
		case button::F3: return "Function3";
		case button::F4: return "Function4";
		case button::F5: return "Function5";
		case button::F6: return "Function6";
		case button::F7: return "Function7";
		case button::F8: return "Function8";
		case button::F9: return "Function9";
		case button::F10: return "Function10";
		case button::F11: return "Function11";
		case button::F12: return "Function12";

		case button::ESC: return "Escape";
		case button::PRINT_SCR: return "PrintScreen";
		case button::PAUSE_BREAK: return "PauseBreak";
		case button::INSERT: return "Insert";
		case button::DELETE: return "Delete";
		case button::HOME: return "Home";
		case button::PAGE_UP: return "PageUp";
		case button::PAGE_DOWN: return "PageDown";
		case button::END: return "End";
		case button::BACKSPACE: return "Backspace";
		case button::RETURN: return "Return";
		case button::ENTER: return "Enter";
		case button::LSHIFT: return "LeftShift";
		case button::RSHIFT: return "RightShift";
		case button::LCTRL: return "LeftControl";
		case button::RCTRL: return "RightControl";
		case button::LALT: return "LeftAlt";
		case button::RALT: return "RightAlt";
		case button::SPACE: return "Spacebar";
		case button::MENU: return "Menu";
		case button::LMETA: return "LeftMeta";
		case button::RMETA: return "RightMeta";
		case button::UP: return "UpArrow";
		case button::LEFT: return "LeftArrow";
		case button::RIGHT: return "RightArrow";
		case button::DOWN: return "DownArrow";
		case button::CAPSLOCK: return "CapsLock";
		case button::TAB: return "Tab";

		case button::NUM_LOCK: return "NumLock";
		case button::NUM_SLASH: return "NumSlash";
		case button::NUM_MULTIPLY: return "NumMultiply";
		case button::NUM_MINUS: return "NumMinus";
		case button::NUM_PLUS: return "NumPlus";
		case button::NUM_DEL: return "NumDelete";
		case button::NUM0: return "Num0";
		case button::NUM1: return "Num1";
		case button::NUM2: return "Num2";
		case button::NUM3: return "Num3";
		case button::NUM4: return "Num4";
		case button::NUM5: return "Num5";
		case button::NUM6: return "Num6";
		case button::NUM7: return "Num7";
		case button::NUM8: return "Num8";
		case button::NUM9: return "Num9";

		case button::JS_A: return "JoystickA";
		case button::JS_B: return "JoystickB";
		case button::JS_Y: return "JoystickY";
		case button::JS_X: return "JoystickX";
		case button::JS_RBUMP: return "JoystickRightBumper";
		case button::JS_LBUMP: return "JoystickLeftBumper";
		case button::JS_PAD_LEFT: return "JoystickDpadLeft";
		case button::JS_PAD_RIGHT: return "JoystickDpadRight";
		case button::JS_PAD_DOWN: return "JoystickDpadDown";
		case button::JS_PAD_UP: return "JoystickDpadUp";
		case button::JS_RSTICK: return "JoystickRightStick";
		case button::JS_LSTICK: return "JoystickLeftStick";
		case button::JS_SELECT: return "JoystickSelect";
		case button::JS_START: return "JoystickStart";
		case button::JS_GAMEPAD: return "JoystickGamepad";

		case button::UNDEFINED: return "Unkown";
	}

	return "UndefinedKey";
}

float win::distance(float x1, float y1, float x2, float y2)
{
	return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}
