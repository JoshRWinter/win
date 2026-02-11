#pragma once

#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <win/Stream.hpp>

#if defined WINPLAT_LINUX
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#elif defined WINPLAT_WINDOWS
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#endif

#ifdef WIN_GL_EXTENSION_STORAGE
#define WIN_STORAGE
#else
#define WIN_STORAGE extern
#endif

namespace win::gl
{

WIN_STORAGE PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
WIN_STORAGE PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;

WIN_STORAGE PFNGLMEMORYBARRIERPROC glMemoryBarrier;
WIN_STORAGE PFNGLFENCESYNCPROC glFenceSync;
WIN_STORAGE PFNGLDELETESYNCPROC glDeleteSync;
WIN_STORAGE PFNGLCLIENTWAITSYNCPROC glClientWaitSync;

WIN_STORAGE PFNGLCREATESHADERPROC glCreateShader;
WIN_STORAGE PFNGLSHADERSOURCEPROC glShaderSource;
WIN_STORAGE PFNGLCOMPILESHADERPROC glCompileShader;
WIN_STORAGE PFNGLGETSHADERIVPROC glGetShaderiv;
WIN_STORAGE PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
WIN_STORAGE PFNGLGETPROGRAMIVPROC glGetProgramiv;
WIN_STORAGE PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
WIN_STORAGE PFNGLATTACHSHADERPROC glAttachShader;
WIN_STORAGE PFNGLDETACHSHADERPROC glDetachShader;
WIN_STORAGE PFNGLLINKPROGRAMPROC glLinkProgram;
WIN_STORAGE PFNGLDELETESHADERPROC glDeleteShader;
WIN_STORAGE PFNGLCREATEPROGRAMPROC glCreateProgram;
WIN_STORAGE PFNGLUSEPROGRAMPROC glUseProgram;
WIN_STORAGE PFNGLDELETEPROGRAMPROC glDeleteProgram;

WIN_STORAGE PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

WIN_STORAGE PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
WIN_STORAGE PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
WIN_STORAGE PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
WIN_STORAGE PFNGLDRAWBUFFERSPROC glDrawBuffers;
WIN_STORAGE PFNGLCOLORMASKIPROC glColorMaski;
WIN_STORAGE PFNGLCLEARBUFFERFVPROC glClearBufferfv;
WIN_STORAGE PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

WIN_STORAGE PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
WIN_STORAGE PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
WIN_STORAGE PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

WIN_STORAGE PFNGLGENBUFFERSPROC glGenBuffers;
WIN_STORAGE PFNGLBINDBUFFERPROC glBindBuffer;
WIN_STORAGE PFNGLBINDBUFFERBASEPROC glBindBufferBase;
WIN_STORAGE PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
WIN_STORAGE PFNGLDELETEBUFFERSPROC glDeleteBuffers;

WIN_STORAGE PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
WIN_STORAGE PFNGLBUFFERDATAPROC glBufferData;
WIN_STORAGE PFNGLBUFFERSUBDATAPROC glBufferSubData;
WIN_STORAGE PFNGLBUFFERSTORAGEPROC glBufferStorage;
WIN_STORAGE PFNGLMAPBUFFERRANGEPROC glMapBufferRange;

WIN_STORAGE PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
WIN_STORAGE PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
WIN_STORAGE PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
WIN_STORAGE PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

WIN_STORAGE PFNGLSHADERSTORAGEBLOCKBINDINGPROC glShaderStorageBlockBinding;
WIN_STORAGE PFNGLGETPROGRAMRESOURCEINDEXPROC glGetProgramResourceIndex;

WIN_STORAGE PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
WIN_STORAGE PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
WIN_STORAGE PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
WIN_STORAGE PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
WIN_STORAGE PFNGLUNIFORM1FPROC glUniform1f;
WIN_STORAGE PFNGLUNIFORM2FPROC glUniform2f;
WIN_STORAGE PFNGLUNIFORM3FPROC glUniform3f;
WIN_STORAGE PFNGLUNIFORM4FPROC glUniform4f;
WIN_STORAGE PFNGLUNIFORM1IPROC glUniform1i;
WIN_STORAGE PFNGLUNIFORM1IPROC glUniform1ui;
WIN_STORAGE PFNGLUNIFORM2IPROC glUniform2i;

WIN_STORAGE PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
WIN_STORAGE PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
WIN_STORAGE PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC glDrawElementsInstancedBaseInstance;
WIN_STORAGE PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
WIN_STORAGE PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;

WIN_STORAGE PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;

#ifdef WINPLAT_WINDOWS
WIN_STORAGE PFNGLTEXIMAGE3DPROC glTexImage3D;
WIN_STORAGE PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
WIN_STORAGE PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

struct DrawElementsIndirectCommand
{
	unsigned count;
	unsigned instance_count;
	unsigned first_index;
	unsigned base_vertex;
	unsigned base_instance;
};

}

namespace win
{

class GLVertexArray
{
	WIN_NO_COPY(GLVertexArray);

public:
	GLVertexArray() { gl::glGenVertexArrays(1, &vao); }

	GLVertexArray(GLVertexArray &&rhs) noexcept
	{
		vao = rhs.vao;
		rhs.vao = -1;
	}

	~GLVertexArray()
	{
		if (vao != -1)
			gl::glDeleteVertexArrays(1, &vao);
	}

	GLVertexArray &operator=(GLVertexArray &&rhs) noexcept
	{
		if (this == &rhs)
			return *this;

		if (vao != -1)
			gl::glDeleteVertexArrays(1, &vao);

		vao = rhs.vao;
		rhs.vao = -1;

		return *this;
	}

	GLuint get() const { return vao; }

private:
	GLuint vao;
};

class GLBuffer
{
	WIN_NO_COPY(GLBuffer);

public:
	GLBuffer() { gl::glGenBuffers(1, &buffer); }

	GLBuffer(GLBuffer &&rhs) noexcept
	{
		buffer = rhs.buffer;
		rhs.buffer = -1;
	}

	~GLBuffer()
	{
		if (buffer != -1)
			gl::glDeleteBuffers(1, &buffer);
	}

	GLBuffer &operator=(GLBuffer &&rhs) noexcept
	{
		if (this == &rhs)
			return *this;

		if (buffer != -1)
			gl::glDeleteBuffers(1, &buffer);

		buffer = rhs.buffer;
		rhs.buffer = -1;

		return *this;
	}

	GLuint get() const { return buffer; }

private:
	GLuint buffer;
};

class GLTexture
{
	WIN_NO_COPY(GLTexture);

public:
	GLTexture() { glGenTextures(1, &texture); }

	GLTexture(GLTexture &&rhs) noexcept
	{
		texture = rhs.texture;
		rhs.texture = -1;
	}

	~GLTexture()
	{
		if (texture != -1)
			glDeleteTextures(1, &texture);
	}

	GLTexture &operator=(GLTexture &&rhs) noexcept
	{
		if (this == &rhs)
			return *this;

		if (texture != -1)
			glDeleteTextures(1, &texture);

		texture = rhs.texture;
		rhs.texture = -1;

		return *this;
	}

	GLuint get() const { return texture; }

private:
	GLuint texture;
};

class GLProgram
{
	WIN_NO_COPY(GLProgram);

public:
	explicit GLProgram(GLuint program)
		: program(program)
	{}

	GLProgram() : program(-1) {}

	GLProgram(GLProgram &&rhs) noexcept
	{
		program = rhs.program;
		rhs.program = -1;
	}

	~GLProgram()
	{
		if (program != -1)
			gl::glDeleteProgram(program);
	}

	GLProgram &operator=(GLProgram &&rhs) noexcept
	{
		if (this == &rhs)
			return *this;

		if (program != -1)
			gl::glDeleteProgram(program);

		program = rhs.program;
		rhs.program = -1;

		return *this;
	}

	GLuint get() const { return program; }

private:
	GLuint program;
};

class GLFramebuffer
{
	WIN_NO_COPY(GLFramebuffer);

public:
	GLFramebuffer()
	{
		gl::glGenFramebuffers(1, &fbo);
	}

	GLFramebuffer(GLFramebuffer &&rhs)
	{
		fbo = rhs.fbo;
		rhs.fbo = -1;
	}

	~GLFramebuffer()
	{
		if (fbo != -1)
			gl::glDeleteFramebuffers(1, &fbo);
	}

	GLFramebuffer &operator=(GLFramebuffer &&rhs)
	{
		if (this == &rhs)
			return *this;

		if (fbo != -1)
			gl::glDeleteFramebuffers(1, &fbo);

		fbo = rhs.fbo;
		rhs.fbo = -1;

		return *this;
	}

	GLuint get() const { return fbo; }

private:
	GLuint fbo;
};

class GLSyncObject
{
	WIN_NO_COPY(GLSyncObject);

public:
	explicit GLSyncObject(GLsync s)
		: sync(s) {}

	GLSyncObject(GLSyncObject &&rhs) noexcept
	{
		sync = rhs.sync;
		rhs.sync = NULL;
	}

	~GLSyncObject()
	{
		if (sync != NULL)
			gl::glDeleteSync(sync);
	}

	GLSyncObject &operator=(GLSyncObject &&rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		if (sync != NULL)
			gl::glDeleteSync(sync);

		sync = rhs.sync;
		rhs.sync = NULL;

		return *this;
	}

	GLsync get() const { return sync; }

private:
	GLsync sync;
};

void gl_check_error();
void gl_load_functions();
GLuint gl_load_shaders(const std::string &, const std::string &);
GLuint gl_load_shaders(Stream, Stream);
GLuint gl_load_compute_shader(Stream);
GLuint gl_load_compute_shader(const std::string &);

}

#endif
