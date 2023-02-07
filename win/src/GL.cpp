#include <win/GL.hpp>

#ifdef WIN_USE_OPENGL

#include <string.h>

namespace win
{

#if defined WINPLAT_LINUX
static void *getproc(const char *name)
{
	void *address = (void*)glXGetProcAddress((const unsigned char*)name);
	if(address == NULL)
		win::bug(std::string("Could not get extension \"") + name + "\"");

	return address;
}
#elif defined WINPLAT_WINDOWS
static void *getproc(const char *name)
{
	void *address = (void*)wglGetProcAddress(name);
	if(address == NULL)
	{
		MessageBox(NULL, (std::string("Missing opengl func: ") + name).c_str(), "Missing Opengl extension", MB_ICONEXCLAMATION);
		std::abort();
	}

	return address;
}
#endif

static void *getproc(const char*);
void load_gl_extensions()
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
	glTexImage3D = (decltype(glTexImage3D))getproc("glTexImage3D");
#endif
}

GLuint load_gl_shaders(const std::string &vertex, const std::string &fragment)
{
	const char *const vertex_cstr = vertex.c_str();
	const char *const fragment_cstr = fragment.c_str();
	const int vertex_cstr_len = vertex.length();
	const int fragment_cstr_len = fragment.length();

	const unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
	const unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vshader, 1, &vertex_cstr, &vertex_cstr_len);
	glShaderSource(fshader, 1, &fragment_cstr, &fragment_cstr_len);

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

const char *v = "#version 330 core\nvoid main(){}";
GLuint load_gl_shaders(Stream vertex, Stream fragment)
{
	return load_gl_shaders(vertex.read_all_as_string(), fragment.read_all_as_string());
}

}

#endif
