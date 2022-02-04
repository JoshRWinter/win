#ifndef WIN_GL_HPP
#define WIN_GL_HPP

#include <win/win.hpp>

#ifdef WIN_USE_OPENGL

#include <win/stream.hpp>

#if defined WINPLAT_LINUX
#include <GL/glx.h>
#elif defined WINPLAT_WINDOWS
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#endif

inline PFNGLCREATESHADERPROC glCreateShader;
inline PFNGLSHADERSOURCEPROC glShaderSource;
inline PFNGLCOMPILESHADERPROC glCompileShader;
inline PFNGLGETSHADERIVPROC glGetShaderiv;
inline PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
inline PFNGLGETPROGRAMIVPROC glGetProgramiv;
inline PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
inline PFNGLATTACHSHADERPROC glAttachShader;
inline PFNGLDETACHSHADERPROC glDetachShader;
inline PFNGLLINKPROGRAMPROC glLinkProgram;
inline PFNGLDELETESHADERPROC glDeleteShader;
inline PFNGLCREATEPROGRAMPROC glCreateProgram;
inline PFNGLUSEPROGRAMPROC glUseProgram;
inline PFNGLDELETEPROGRAMPROC glDeleteProgram;
inline PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
inline PFNGLGENBUFFERSPROC glGenBuffers;
inline PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
inline PFNGLBINDBUFFERPROC glBindBuffer;
inline PFNGLBUFFERDATAPROC glBufferData;
inline PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
inline PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
inline PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
inline PFNGLDELETEBUFFERSPROC glDeleteBuffers;
inline PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
inline PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
inline PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
inline PFNGLUNIFORM1FPROC glUniform1f;
inline PFNGLUNIFORM2FPROC glUniform2f;
inline PFNGLUNIFORM4FPROC glUniform4f;
inline PFNGLUNIFORM1IPROC glUniform1i;
inline PFNGLUNIFORM2IPROC glUniform2i;
inline PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
inline PFNGLBUFFERSUBDATAPROC glBufferSubData;

#if defined WINPLAT_LINUX
inline PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
#elif defined WINPLAT_WINDOWS
inline PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
inline PFNGLTEXIMAGE3DPROC glTexImage3D;
#endif

namespace win
{

GLuint load_gl_shaders(const std::string&, const std::string&);
GLuint load_gl_shaders(Stream, Stream);
void load_gl_extensions();

}



#endif

#endif
