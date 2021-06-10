#ifndef WIN_EVENT_H
#define WIN_EVENT_H

#include <string.h>

#if defined WINPLAT_WINDOWS
#undef DELETE // lol windows.h
#endif

namespace win
{

enum class Button
{
	UNDEFINED,

	// mice
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE4,
	MOUSE5,
	MOUSE6,
	MOUSE7,

	// letters
	A, B, C, D,
	E, F, G, H,
	I, J, K, L,
	M, N, O, P,
	Q, R, S, T,
	U, V, W, X,
	Y, Z,

	// numbers (top row)
	D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

	// symbols
	BACKTICK, DASH, EQUALS, LBRACKET,
	RBRACKET, SEMICOLON, APOSTROPHE,
	COMMA, PERIOD, SLASH, BACKSLASH,

	// function keys
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	// misc
	ESC, PRINT_SCR, PAUSE_BREAK, INSERT, DELETE,
	HOME, PAGE_UP, PAGE_DOWN, END, BACKSPACE,
	RETURN, ENTER, LSHIFT, RSHIFT, LCTRL, RCTRL,
	LALT, RALT, SPACE, MENU, LMETA, RMETA, UP,
	LEFT, RIGHT, DOWN, CAPSLOCK, TAB,

	// numpad
	NUM_LOCK, NUM_SLASH, NUM_MULTIPLY, NUM_MINUS,
	NUM_PLUS, NUM_DEL,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5,
	NUM6, NUM7, NUM8, NUM9,
};

}

#endif
