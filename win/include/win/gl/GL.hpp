#pragma once

#include <win/Win.hpp>

#ifdef WIN_USE_OPENGL

#include <win/Stream.hpp>

#if defined WINPLAT_LINUX
#include <GL/glx.h>
#include <GL/glext.h>
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

WIN_STORAGE PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
WIN_STORAGE PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
WIN_STORAGE PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

WIN_STORAGE PFNGLGENBUFFERSPROC glGenBuffers;
WIN_STORAGE PFNGLBINDBUFFERPROC glBindBuffer;
WIN_STORAGE PFNGLDELETEBUFFERSPROC glDeleteBuffers;

WIN_STORAGE PFNGLBUFFERDATAPROC glBufferData;
WIN_STORAGE PFNGLBUFFERSTORAGEPROC glBufferStorage;
WIN_STORAGE PFNGLMAPBUFFERRANGEPROC glMapBufferRange;

WIN_STORAGE PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
WIN_STORAGE PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
WIN_STORAGE PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

WIN_STORAGE PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
WIN_STORAGE PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
WIN_STORAGE PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
WIN_STORAGE PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
WIN_STORAGE PFNGLUNIFORM1FPROC glUniform1f;
WIN_STORAGE PFNGLUNIFORM2FPROC glUniform2f;
WIN_STORAGE PFNGLUNIFORM4FPROC glUniform4f;
WIN_STORAGE PFNGLUNIFORM1IPROC glUniform1i;
WIN_STORAGE PFNGLUNIFORM2IPROC glUniform2i;

WIN_STORAGE PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
WIN_STORAGE PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
WIN_STORAGE PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;

struct DrawElementsIndirectCommand
{
	unsigned count;
	unsigned instance_count;
	unsigned first_index;
	unsigned base_vertex;
	unsigned base_instance;
};

#if defined WINPLAT_LINUX
inline PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
#elif defined WINPLAT_WINDOWS
inline PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
inline PFNGLTEXIMAGE3DPROC glTexImage3D;
#endif

}

namespace win
{

void gl_check_error();
GLuint load_gl_shaders(const std::string&, const std::string&);
GLuint load_gl_shaders(Stream, Stream);
void load_gl_extensions();

}

#endif
