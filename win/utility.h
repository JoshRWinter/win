#ifndef WIN_UTILITY_H
#define WIN_UTILITY_H

#include <GL/glext.h>

#ifdef WIN_STORAGE
#define WIN_EXTERN
#else
#define WIN_EXTERN extern
#endif

WIN_EXTERN PFNGLCREATESHADERPROC glCreateShader;
WIN_EXTERN PFNGLSHADERSOURCEPROC glShaderSource;
WIN_EXTERN PFNGLCOMPILESHADERPROC glCompileShader;
WIN_EXTERN PFNGLGETSHADERIVPROC glGetShaderiv;
WIN_EXTERN PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
WIN_EXTERN PFNGLATTACHSHADERPROC glAttachShader;
WIN_EXTERN PFNGLLINKPROGRAMPROC glLinkProgram;
WIN_EXTERN PFNGLDELETESHADERPROC glDeleteShader;
WIN_EXTERN PFNGLCREATEPROGRAMPROC glCreateProgram;
WIN_EXTERN PFNGLUSEPROGRAMPROC glUseProgram;
WIN_EXTERN PFNGLDELETEPROGRAMPROC glDeleteProgram;
WIN_EXTERN PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
WIN_EXTERN PFNGLGENBUFFERSPROC glGenBuffers;
WIN_EXTERN PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
WIN_EXTERN PFNGLBINDBUFFERPROC glBindBuffer;
WIN_EXTERN PFNGLBUFFERDATAPROC glBufferData;
WIN_EXTERN PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
WIN_EXTERN PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
WIN_EXTERN PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
WIN_EXTERN PFNGLDELETEBUFFERSPROC glDeleteBuffers;
WIN_EXTERN PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
WIN_EXTERN PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
WIN_EXTERN PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
WIN_EXTERN PFNGLUNIFORM1FPROC glUniform1f;
WIN_EXTERN PFNGLUNIFORM2FPROC glUniform2f;
WIN_EXTERN PFNGLUNIFORM4FPROC glUniform4f;
WIN_EXTERN PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
WIN_EXTERN PFNGLBUFFERSUBDATAPROC glBufferSubData;

#if defined WINPLAT_LINUX
WIN_EXTERN PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
#endif

namespace win
{
	struct color
	{
		constexpr color(float r, float g, float b, float a = 1.0f)
			: red(r), green(g), blue(b), alpha(a) {}
		constexpr color(int r, int g, int b, int a = 255.0f)
			: red(r / 255.0f), green(g / 255.0f), blue(b / 255.0f), alpha(a / 255.0f) {}

		float red, green, blue, alpha;
	};

	void load_extensions();
	unsigned load_shaders(const char*, const char*);
	void init_ortho(float *matrix, float, float, float, float);
	const char *key_name(button);
}

#endif
