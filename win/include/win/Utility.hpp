#pragma once

#include <type_traits>

#include <win/Event.hpp>
#include <win/Stream.hpp>

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

template <typename T> struct Area
{
	Area()
		: left(0), right(0), bottom(0), top(0) {}
	Area(const typename std::enable_if<std::is_fundamental<T>::value, T>::type left, const T right, const T bottom, const T top)
		: left(left), right(right), bottom(bottom), top(top) {}

	T left, right, bottom, top;
};

template <typename T> struct Dimensions
{
	Dimensions()
		: width(0), height(0) {}
	Dimensions(const typename std::enable_if<std::is_fundamental<T>::value, T>::type width, const T height)
		: width(width), height(height) {}

	T width, height;
};

template <typename T> struct Pair
{
	Pair() : x(0), y(0) {}
	Pair(const typename std::enable_if<std::is_fundamental<T>::value, T>::type x, const T y) : x(x), y(y) {}

	T x;
	T y;
};

const char *key_name(Button);

}
