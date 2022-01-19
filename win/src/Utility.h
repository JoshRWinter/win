#ifndef WIN_UTILITY_H
#define WIN_UTILITY_H

#include <vector>
#include <string>

#if defined WINPLAT_LINUX
#include <GL/glext.h>
#elif defined WINPLAT_WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#endif

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
WIN_EXTERN PFNGLGETPROGRAMIVPROC glGetProgramiv;
WIN_EXTERN PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
WIN_EXTERN PFNGLATTACHSHADERPROC glAttachShader;
WIN_EXTERN PFNGLDETACHSHADERPROC glDetachShader;
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
WIN_EXTERN PFNGLUNIFORM1IPROC glUniform1i;
WIN_EXTERN PFNGLUNIFORM2IPROC glUniform2i;
WIN_EXTERN PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
WIN_EXTERN PFNGLBUFFERSUBDATAPROC glBufferSubData;

#if defined WINPLAT_LINUX
WIN_EXTERN PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
#elif defined WINPLAT_WINDOWS
WIN_EXTERN PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
WIN_EXTERN PFNGLTEXIMAGE3DPROC glTexImage3D;
#endif

namespace win
{

struct Color
{
	constexpr Color()
		: red(0.5f), green(0.5f), blue(0.5f), alpha(1.0f) {}
	constexpr Color(float r, float g, float b, float a = 1.0f)
		: red(r), green(g), blue(b), alpha(a) {}
	constexpr Color(int r, int g, int b, int a = 255.0f)
		: red(r / 255.0f), green(g / 255.0f), blue(b / 255.0f), alpha(a / 255.0f) {}

	float red, green, blue, alpha;
};

struct Area
{
	Area()
		: left(-1.0f), right(1.0f), bottom(1.0f), top(-1.0f) {}
	Area(const float l, const float r, const float b, const float t)
		: left(l), right(r), bottom(b), top(t) {}

	float left, right, bottom, top;
};

struct Point
{
	Point() : x(0.0f), y(0.0f) {}
	Point(float xx, float yy) : x(xx), y(yy) {}

	float x;
	float y;
};

struct Program
{
	Program();
	Program(GLuint);
	Program(const Program&) = delete;
	Program(Program&&);
	~Program();

	void operator=(const Program&) = delete;
	Program &operator=(Program&&);

	GLuint get() const;
	GLuint object;
};

struct Vao
{
	Vao();
	Vao(const Vao&) = delete;
	Vao(Vao&&);
	~Vao();

	void operator=(const Vao&) = delete;
	Vao &operator=(Vao&&);

	GLuint get() const;
	GLuint object;
};

struct Vbo
{
	Vbo();
	Vbo(const Vbo&) = delete;
	Vbo(Vbo&&);
	~Vbo();

	void operator=(const Vbo&) = delete;
	Vbo &operator=(Vbo&&);

	GLuint get() const;
	GLuint object;
};

void load_extensions();
unsigned load_shaders(const char*, int, const char*, int);
unsigned load_shaders(const char*, const char*);
unsigned load_shaders(AssetRollStream, AssetRollStream);
void init_ortho(float *matrix, float, float, float, float);
const char *key_name(Button);

// random useful utilities
float distance(float, float, float, float);
float align(float, float, float);
float angle_diff(float, float);
float zerof(float, float);

}

#endif
