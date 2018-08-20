#ifndef WIN_EVENT_H
#define WIN_EVENT_H

#include <string.h>

namespace win
{

#if defined WINPLAT_LINUX
enum class button
{
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
	D0, D1, D2, D3, D4, D5, D6, D7, D8, D9

	// symbols
	TILDE, DASH, EQUALS, LBRACKET,
	RBRACKET, SEMICOLON, APOSTROPHE,
	COMMA, PERIOD, SLASH, BACKSLASH,

	// function keys
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	// misc
	ESC, PRINT_SCR, PAUSE_BREAK, INSERT, DELETE,
	HOME, PAGE_UP, PAGE_DOWN, END, BACKSPACE,
	RETURN, ENTER, LSHIFT, RSHIFT, LCTRL, RCTRL,
	LALT, RALT, SPACE, MENU, LMETA, UP, LEFT,
	RIGHT, DOWN, CAPSLOCK, TAB,

	// numpad
	NUM_LOCK, NUM_SLASH, NUM_MULTIPLY, NUM_MINUS,
	NUM_PLUS, NUM_DEL,
	NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5,
	NUM_6, NUM_7, NUM_8, NUM_9
};

class button
{
public:
	// mice
	static constexpr const char *MOUSE_LEFT = "MS1\0";
	static constexpr const char *MOUSE_RIGHT = "MS2\0";
	static constexpr const char *MOUSE_MIDDLE = "MS3\0";
	static constexpr const char *MOUSE4 = "MS4\0";
	static constexpr const char *MOUSE5 = "MS5\0";
	static constexpr const char *MOUSE6 = "MS6\0";
	static constexpr const char *MOUSE7 = "MS7\0";

	// letters
	static constexpr const char *A = "AC01";
	static constexpr const char *B = "AB05";
	static constexpr const char *C = "AB03";
	static constexpr const char *D = "AC03";
	static constexpr const char *E = "AD03";
	static constexpr const char *F = "AC04";
	static constexpr const char *G = "AC05";
	static constexpr const char *H = "AC06";
	static constexpr const char *I = "AD08";
	static constexpr const char *J = "AC07";
	static constexpr const char *K = "AC08";
	static constexpr const char *L = "AC09";
	static constexpr const char *M = "AB07";
	static constexpr const char *N = "AB06";
	static constexpr const char *O = "AD09";
	static constexpr const char *P = "AD10";
	static constexpr const char *Q = "AD01";
	static constexpr const char *R = "AD04";
	static constexpr const char *S = "AC02";
	static constexpr const char *T = "AD05";
	static constexpr const char *U = "AD07";
	static constexpr const char *V = "AB04";
	static constexpr const char *W = "AD02";
	static constexpr const char *X = "AB02";
	static constexpr const char *Y = "AD06";
	static constexpr const char *Z = "AB01";

	// digits, top row numbers (not num pad)
	static constexpr const char *D0 = "AE10";
	static constexpr const char *D1 = "AE01";
	static constexpr const char *D2 = "AE02";
	static constexpr const char *D3 = "AE03";
	static constexpr const char *D4 = "AE04";
	static constexpr const char *D5 = "AE05";
	static constexpr const char *D6 = "AE06";
	static constexpr const char *D7 = "AE07";
	static constexpr const char *D8 = "AE08";
	static constexpr const char *D9 = "AE09";

	// symbols
	static constexpr const char *TILDE = "TLDE";
	static constexpr const char *DASH = "AE11";
	static constexpr const char *EQUALS = "AE12";
	static constexpr const char *LBRACKET = "AD11";
	static constexpr const char *RBRACKET = "AD12";
	static constexpr const char *SEMICOLON = "AC10";
	static constexpr const char *APOSTROPHE = "AC11";
	static constexpr const char *COMMA = "AB08";
	static constexpr const char *PERIOD = "AB09";
	static constexpr const char *SLASH = "AB10";
	static constexpr const char *BACKSLASH = "BKSL";

	// function keys
	static constexpr const char *F1 = "FK01";
	static constexpr const char *F2 = "FK02";
	static constexpr const char *F3 = "FK03";
	static constexpr const char *F4 = "FK04";
	static constexpr const char *F5 = "FK05";
	static constexpr const char *F6 = "FK06";
	static constexpr const char *F7 = "FK07";
	static constexpr const char *F8 = "FK08";
	static constexpr const char *F9 = "FK09";
	static constexpr const char *F10 = "FK10";
	static constexpr const char *F11 = "FK11";
	static constexpr const char *F12 = "FK12";

	// misc
	static constexpr const char *ESC = "ESC\0";
	static constexpr const char *PRINT_SCR = "PRSC";
	static constexpr const char *PAUSE_BREAK = "PAUS";
	static constexpr const char *INSERT = "INS\0";
	static constexpr const char *DELETE = "DELE";
	static constexpr const char *HOME = "HOME";
	static constexpr const char *PAGE_UP = "PGUP";
	static constexpr const char *PAGE_DOWN = "PGDN";
	static constexpr const char *END = "END\0";
	static constexpr const char *BACKSPACE = "BKSP";
	static constexpr const char *RETURN = "RTRN";
	static constexpr const char *ENTER = "KPEN";
	static constexpr const char *LSHIFT = "LFSH";
	static constexpr const char *RSHIFT = "RTSH";
	static constexpr const char *LCTRL = "LCTL";
	static constexpr const char *RCTRL = "RCTL";
	static constexpr const char *LALT = "LALT";
	static constexpr const char *RALT = "RALT";
	static constexpr const char *SPACE = "SPCE";
	static constexpr const char *MENU = "COMP";
	static constexpr const char *LMETA = "LWIN"; // "windows key
	static constexpr const char *UP = "UP\0\0";
	static constexpr const char *LEFT = "LEFT";
	static constexpr const char *RIGHT = "RGHT";
	static constexpr const char *DOWN = "DOWN";
	static constexpr const char *CAPSLOCK = "CAPS";
	static constexpr const char *TAB = "TAB\0";

	// numpad
	static constexpr const char *NUM_LOCK = "NMLK";
	static constexpr const char *NUM_SLASH = "KPDV";
	static constexpr const char *NUM_MULTIPLY = "KPMU";
	static constexpr const char *NUM_MINUS = "KPSU";
	static constexpr const char *NUM_PLUS = "KPAD";
	static constexpr const char *NUM_DEL = "KPDL";
	static constexpr const char *NUM_0 = "KP0";
	static constexpr const char *NUM_1 = "KP1";
	static constexpr const char *NUM_2 = "KP2";
	static constexpr const char *NUM_3 = "KP3";
	static constexpr const char *NUM_4 = "KP4";
	static constexpr const char *NUM_5 = "KP5";
	static constexpr const char *NUM_6 = "KP6";
	static constexpr const char *NUM_7 = "KP7";
	static constexpr const char *NUM_8 = "KP8";
	static constexpr const char *NUM_9 = "KP9";

	button(const char *const n) : name(n) {}

	void print() const
	{
		for(int i = 0; i < XkbKeyNameLength && name[i] != 0; ++i)
			std::cerr << name[i];
		std::cerr << ", ";
	}

	bool operator==(const char *rhs) const
	{
		if(strlen(rhs) > XkbKeyNameLength)
			return false;

		for(int i = 0; i < XkbKeyNameLength && name[i] != 0; ++i)
			if(name[i] != rhs[i])
				return false;

		return true;
	};

private:
	// NOT NULL TERMINATED (taken straight from xkb X11 extension)
	const char *const name;
};
#endif

}

#endif
