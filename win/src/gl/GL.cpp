#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#define WIN_GL_EXTENSION_STORAGE
#include <win/gl/GL.hpp>

using namespace win::gl;

namespace win
{

#if defined WINPLAT_LINUX
static void *get_proc(const char *name)
{
	void *address = (void*)glXGetProcAddress((const unsigned char*) name);

	return address;
}
#elif defined WINPLAT_WINDOWS
static void *get_proc(const char *name)
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

void gl_check_error()
{
	const char *errname;
	const auto err = glGetError();

	switch (err)
	{
		case GL_NO_ERROR:
			errname = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			errname = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errname = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			errname = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			errname = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			errname = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			errname = "GL_OUT_OF_MEMORY";
			break;
		default:
			errname = "Unknown gl error";
			break;
	}
	if (err != GL_NO_ERROR)
		win::bug("GL error " + std::string(errname) + " (" + std::to_string(err) + ")");
}

void gl_load_functions()
{
	glDebugMessageCallback = (decltype(glDebugMessageCallback))get_proc("glDebugMessageCallback");
	glDebugMessageControl = (decltype(glDebugMessageControl))get_proc("glDebugMessageControl");

	glMemoryBarrier = (decltype(glMemoryBarrier))get_proc("glMemoryBarrier");
	glFenceSync = (decltype(glFenceSync))get_proc("glFenceSync");
	glDeleteSync = (decltype(glDeleteSync))get_proc("glDeleteSync");
	glClientWaitSync = (decltype(glClientWaitSync))get_proc("glClientWaitSync");

	glCreateShader = (decltype(glCreateShader)) get_proc("glCreateShader");
	glShaderSource = (decltype(glShaderSource)) get_proc("glShaderSource");
	glCompileShader = (decltype(glCompileShader)) get_proc("glCompileShader");
	glGetShaderiv = (decltype(glGetShaderiv)) get_proc("glGetShaderiv");
	glGetShaderInfoLog = (decltype(glGetShaderInfoLog)) get_proc("glGetShaderInfoLog");
	glGetProgramiv = (decltype(glGetProgramiv)) get_proc("glGetProgramiv");
	glGetProgramInfoLog = (decltype(glGetProgramInfoLog)) get_proc("glGetProgramInfoLog");
	glAttachShader = (decltype(glAttachShader)) get_proc("glAttachShader");
	glDetachShader = (decltype(glDetachShader)) get_proc("glDetachShader");
	glLinkProgram = (decltype(glLinkProgram)) get_proc("glLinkProgram");
	glDeleteShader = (decltype(glDeleteShader)) get_proc("glDeleteShader");
	glCreateProgram = (decltype(glCreateProgram)) get_proc("glCreateProgram");
	glUseProgram = (decltype(glUseProgram)) get_proc("glUseProgram");
	glDeleteProgram = (decltype(glDeleteProgram)) get_proc("glDeleteProgram");

	glGenerateMipmap = (decltype(glGenerateMipmap)) get_proc("glGenerateMipmap");

	glGenFramebuffers = (decltype(glGenFramebuffers)) get_proc("glGenFramebuffers");
	glBindFramebuffer = (decltype(glBindFramebuffer)) get_proc("glBindFramebuffer");
	glFramebufferTexture2D = (decltype(glFramebufferTexture2D)) get_proc("glFramebufferTexture2D");
	glDrawBuffers = (decltype(glDrawBuffers)) get_proc("glDrawBuffers");
	glColorMaski = (decltype(glColorMaski)) get_proc("glColorMaski");
	glClearBufferfv = (decltype(glClearBufferfv)) get_proc("glClearBufferfv");
	glDeleteFramebuffers = (decltype(glDeleteFramebuffers)) get_proc("glDeleteFramebuffers");

	glGenVertexArrays = (decltype(glGenVertexArrays)) get_proc("glGenVertexArrays");
	glBindVertexArray = (decltype(glBindVertexArray)) get_proc("glBindVertexArray");
	glDeleteVertexArrays = (decltype(glDeleteVertexArrays)) get_proc("glDeleteVertexArrays");

	glGenBuffers = (decltype(glGenBuffers)) get_proc("glGenBuffers");
	glBindBuffer = (decltype(glBindBuffer)) get_proc("glBindBuffer");
	glBindBufferBase = (decltype(glBindBufferBase)) get_proc("glBindBufferBase");
	glBindBufferRange = (decltype(glBindBufferRange)) get_proc("glBindBufferRange");
	glDeleteBuffers = (decltype(glDeleteBuffers)) get_proc("glDeleteBuffers");

	glGetBufferSubData = (decltype(glGetBufferSubData)) get_proc("glGetBufferSubData");
	glBufferData = (decltype(glBufferData)) get_proc("glBufferData");
	glBufferSubData = (decltype(glBufferSubData)) get_proc("glBufferSubData");
	glBufferStorage = (decltype(glBufferStorage)) get_proc("glBufferStorage");
	glMapBufferRange = (decltype(glMapBufferRange)) get_proc("glMapBufferRange");

	glVertexAttribDivisor = (decltype(glVertexAttribDivisor)) get_proc("glVertexAttribDivisor");
	glVertexAttribPointer = (decltype(glVertexAttribPointer)) get_proc("glVertexAttribPointer");
	glVertexAttribIPointer = (decltype(glVertexAttribIPointer)) get_proc("glVertexAttribIPointer");
	glEnableVertexAttribArray = (decltype(glEnableVertexAttribArray)) get_proc("glEnableVertexAttribArray");

	glShaderStorageBlockBinding = (decltype(glShaderStorageBlockBinding)) get_proc("glShaderStorageBlockBinding");
	glGetProgramResourceIndex = (decltype(glGetProgramResourceIndex)) get_proc("glGetProgramResourceIndex");

	glGetUniformBlockIndex = (decltype(glGetUniformBlockIndex)) get_proc("glGetUniformBlockIndex");
	glUniformBlockBinding = (decltype(glUniformBlockBinding)) get_proc("glUniformBlockBinding");
	glGetUniformLocation = (decltype(glGetUniformLocation)) get_proc("glGetUniformLocation");
	glUniformMatrix4fv = (decltype(glUniformMatrix4fv)) get_proc("glUniformMatrix4fv");
	glUniform1f = (decltype(glUniform1f)) get_proc("glUniform1f");
	glUniform2f = (decltype(glUniform2f)) get_proc("glUniform2f");
	glUniform3f = (decltype(glUniform3f)) get_proc("glUniform3f");
	glUniform4f = (decltype(glUniform4f)) get_proc("glUniform4f");
	glUniform1i = (decltype(glUniform1i)) get_proc("glUniform1i");
	glUniform1ui = (decltype(glUniform1ui)) get_proc("glUniform1ui");
	glUniform2i = (decltype(glUniform2i)) get_proc("glUniform2i");

	glDrawElementsInstanced = (decltype(glDrawElementsInstanced)) get_proc("glDrawElementsInstanced");
	glDrawElementsInstancedBaseInstance = (decltype(glDrawElementsInstancedBaseInstance)) get_proc("glDrawElementsInstancedBaseInstance");
	glDrawElementsBaseVertex = (decltype(glDrawElementsBaseVertex)) get_proc("glDrawElementsBaseVertex");
	glMultiDrawElementsIndirect = (decltype(glMultiDrawElementsIndirect)) get_proc("glMultiDrawElementsIndirect");
	glDrawArraysInstanced = (decltype(glDrawArraysInstanced)) get_proc("glDrawArraysInstanced");

	glDispatchCompute = (decltype(glDispatchCompute)) get_proc("glDispatchCompute");

#ifdef WINPLAT_WINDOWS
	glTexImage3D = (decltype(glTexImage3D)) get_proc("glTexImage3D");
	glTexSubImage3D = (decltype(glTexSubImage3D)) get_proc("glTexSubImage3D");
	glActiveTexture = (decltype(glActiveTexture)) get_proc("glActiveTexture");
#endif
}

GLuint gl_load_shaders(const std::string &vertex, const std::string &fragment)
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

GLuint gl_load_shaders(Stream vertex, Stream fragment)
{
	return gl_load_shaders(vertex.read_all_as_string(), fragment.read_all_as_string());
}

GLuint gl_load_compute_shader(const std::string &compute)
{
	const char *const compute_cstr = compute.c_str();
	const int compute_cstr_len = compute.length();

	const unsigned cshader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(cshader, 1, &compute_cstr, &compute_cstr_len);

	glCompileShader(cshader);

	int success = 1;
	char buffer[2000] = "";
	glGetShaderiv(cshader, GL_COMPILE_STATUS, &success);
	if (success == 0)
	{
		glGetShaderInfoLog(cshader, 2000, NULL, buffer);
		win::bug(std::string("compute shader:\n") + buffer);
	}

	unsigned program = glCreateProgram();
	glAttachShader(program, cshader);
	glLinkProgram(program);
	GLint linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		glGetProgramInfoLog(program, 2000, NULL, buffer);
		win::bug(std::string("linker:\n") + buffer);
	}
	glDetachShader(program, cshader);
	glDeleteShader(cshader);

	return program;
}

GLuint gl_load_compute_shader(Stream compute)
{
	return gl_load_compute_shader(compute.read_all_as_string());
}

}

#endif
