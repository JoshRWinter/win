#pragma once

namespace win
{

enum class Button
{
	undefined,

	// mice
	mouse_left,
	mouse_right,
	mouse_middle,
	mouse4,
	mouse5,
	mouse6,
	mouse7,

	// letters
	a, b, c, d,
	e, f, g, h,
	i, j, k, l,
	m, n, o, p,
	q, r, s, t,
	u, v, w, x,
	y, z,

	// numbers (top row)
	d0, d1, d2, d3, d4, d5, d6, d7, d8, d9,

	// symbols
	backtick, dash, equals, lbracket,
	rbracket, semicolon, apostrophe,
	comma, period, slash, backslash,

	// function keys
	f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,

	// misc
	esc, print_scr, pause_break, insert, del,
	home, page_up, page_down, end, backspace,
	ret, enter, lshift, rshift, lctrl, rctrl,
	lalt, ralt, space, menu, lmeta, rmeta, up,
	left, right, down, capslock, tab,

	// numpad
	num_lock, num_slash, num_multiply, num_minus,
	num_plus, num_del,
	num0, num1, num2, num3, num4, num5,
	num6, num7, num8, num9,
};

enum class WindowEvent
{
	close
};

}
