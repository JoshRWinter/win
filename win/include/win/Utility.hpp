#pragma once

#include <type_traits>

#include <win/Event.hpp>
#include <win/Stream.hpp>

namespace win
{

template <typename T> struct Color
{
	static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Color - must be an integer or real type");

	Color()
	{
		const T v = (T)(std::is_integral<T>::value ? 255 : 1.0);

		red = v;
		green = v;
		blue = v;
		alpha = v;
	}

	Color(T r, T g, T b)
		: red(r), green(g), blue(b)
	{
		alpha = (T)(std::is_integral<T>::value ? 255 : 1.0);
	}

	Color(T r, T g, T b, T a)
		: red(r), green(g), blue(b), alpha(a) {}

	bool operator==(const Color<T> &rhs) const { return red == rhs.red && green == rhs.green && blue == rhs.blue; }
	bool operator!=(const Color<T> &rhs) const { return red != rhs.red || green != rhs.green || blue != rhs.blue; }

	T red, green, blue, alpha;
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
