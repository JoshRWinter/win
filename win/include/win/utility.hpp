#ifndef WIN_UTILITY_HPP
#define WIN_UTILITY_HPP

#include <win/event.hpp>
#include <win/stream.hpp>

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

struct FScreenArea
{
	FScreenArea()
		: left(0.0f), right(0.0f), bottom(0.0f), top(0.0f) {}
	FScreenArea(const float left, const float right, const float bottom, const float top)
		: left(left), right(right), bottom(bottom), top(top) {}

	float left, right, bottom, top;
};

struct IDimensions2D
{
	IDimensions2D()
		: width(0), height(0) {}
	IDimensions2D(const int width, const int height)
		: width(width), height(height) {}

	int width, height;
};

struct FPair
{
	FPair() : x(0.0f), y(0.0f) {}
	FPair(const float x, const float y) : x(x), y(y) {}

	float x;
	float y;
};

struct IPair
{
	IPair() : x(0), y(0) {}
	IPair(const int x, const int y) : x(x), y(y) {}

	int x;
	int y;
};

const char *key_name(Button);

// random useful utilities
float distance(float, float, float, float);
float align(float, float, float);
float angle_diff(float, float);
float zerof(float, float);

}

#endif
