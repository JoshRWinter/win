#ifndef WIN_EVENT_H
#define WIN_EVENT_H

#include <string.h>

namespace win
{

// mouse buttons
enum class mouse
{
};

#if defined WINPLAT_LINUX
class pkey
{
public:
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

	pkey(const char *const n) : name(n) {}

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
