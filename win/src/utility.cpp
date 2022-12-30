#define _USE_MATH_DEFINES
#include <cmath>

#include <win/utility.hpp>

namespace win
{

const char *key_name(const Button key)
{
	switch(key)
	{
	case Button::mouse_left: return "LeftMouse";
	case Button::mouse_right: return "RightMouse";
	case Button::mouse_middle: return "MiddleMouse";
	case Button::mouse4: return "Mouse4";
	case Button::mouse5: return "Mouse5";
	case Button::mouse6: return "Mouse6";
	case Button::mouse7: return "Mouse7";

	case Button::a: return "A";
	case Button::b: return "B";
	case Button::c: return "C";
	case Button::d: return "D";
	case Button::e: return "E";
	case Button::f: return "F";
	case Button::g: return "G";
	case Button::h: return "H";
	case Button::i: return "I";
	case Button::j: return "J";
	case Button::k: return "K";
	case Button::l: return "L";
	case Button::m: return "M";
	case Button::n: return "N";
	case Button::o: return "O";
	case Button::p: return "P";
	case Button::q: return "Q";
	case Button::r: return "R";
	case Button::s: return "S";
	case Button::t: return "T";
	case Button::u: return "U";
	case Button::v: return "V";
	case Button::w: return "W";
	case Button::x: return "X";
	case Button::y: return "Y";
	case Button::z: return "Z";

	case Button::d0: return "0";
	case Button::d1: return "1";
	case Button::d2: return "2";
	case Button::d3: return "3";
	case Button::d4: return "4";
	case Button::d5: return "5";
	case Button::d6: return "6";
	case Button::d7: return "7";
	case Button::d8: return "8";
	case Button::d9: return "9";

	case Button::backtick: return "BackTick";
	case Button::dash: return "Dash";
	case Button::equals: return "Equals";
	case Button::lbracket: return "LeftBracket";
	case Button::rbracket: return "RightBracket";
	case Button::semicolon: return "Semicolon";
	case Button::apostrophe: return "Apostrophe";
	case Button::comma: return "Comma";
	case Button::period: return "Period";
	case Button::slash: return "Slash";
	case Button::backslash: return "BackSlash";

	case Button::f1: return "Function1";
	case Button::f2: return "Function2";
	case Button::f3: return "Function3";
	case Button::f4: return "Function4";
	case Button::f5: return "Function5";
	case Button::f6: return "Function6";
	case Button::f7: return "Function7";
	case Button::f8: return "Function8";
	case Button::f9: return "Function9";
	case Button::f10: return "Function10";
	case Button::f11: return "Function11";
	case Button::f12: return "Function12";

	case Button::esc: return "Escape";
	case Button::print_scr: return "PrintScreen";
	case Button::pause_break: return "PauseBreak";
	case Button::insert: return "Insert";
	case Button::del: return "Delete";
	case Button::home: return "Home";
	case Button::page_up: return "PageUp";
	case Button::page_down: return "PageDown";
	case Button::end: return "End";
	case Button::backspace: return "Backspace";
	case Button::ret: return "Return";
	case Button::enter: return "Enter";
	case Button::lshift: return "LeftShift";
	case Button::rshift: return "RightShift";
	case Button::lctrl: return "LeftControl";
	case Button::rctrl: return "RightControl";
	case Button::lalt: return "LeftAlt";
	case Button::ralt: return "RightAlt";
	case Button::space: return "Spacebar";
	case Button::menu: return "Menu";
	case Button::lmeta: return "LeftMeta";
	case Button::rmeta: return "RightMeta";
	case Button::up: return "UpArrow";
	case Button::left: return "LeftArrow";
	case Button::right: return "RightArrow";
	case Button::down: return "DownArrow";
	case Button::capslock: return "CapsLock";
	case Button::tab: return "Tab";

	case Button::num_lock: return "NumLock";
	case Button::num_slash: return "NumSlash";
	case Button::num_multiply: return "NumMultiply";
	case Button::num_minus: return "NumMinus";
	case Button::num_plus: return "NumPlus";
	case Button::num_del: return "NumDelete";
	case Button::num0: return "Num0";
	case Button::num1: return "Num1";
	case Button::num2: return "Num2";
	case Button::num3: return "Num3";
	case Button::num4: return "Num4";
	case Button::num5: return "Num5";
	case Button::num6: return "Num6";
	case Button::num7: return "Num7";
	case Button::num8: return "Num8";
	case Button::num9: return "Num9";

	case Button::undefined: return "Unkown";
	}

	return "UndefinedKey";
}

float distance(float x1, float y1, float x2, float y2)
{
	return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
}

static float normalize(float a)
{
	const float PI2 = 2.0f * M_PI;

	while(a > PI2)
		a -= PI2;

	while(a < 0.0f)
		a += PI2;

	return a;
}

float align(float angle, float target, const float step)
{
	target = normalize(target);
	angle = normalize(angle);

	if(target > angle)
	{
		if(target - angle > M_PI)
		{
			angle -= step;

			if(angle < 0.0f)
			{
				angle += 2.0f * M_PI;

				if(angle < target)
					angle = target;
			}
		}
		else
		{
			angle += step;

			if(angle > target)
				angle = target;
		}
	}
	else
	{
		if(angle - target > M_PI)
		{
			angle += step;

			if(angle > 2.0f * M_PI)
			{
				angle -= 2.0f * M_PI;

				if(angle > target)
					angle = target;
			}
		}
		else
		{
			angle -= step;

			if(angle < target)
				angle = target;
		}
	}

	return angle;
}

float angle_diff(float a, float b)
{
	a = normalize(a);
	b = normalize(b);

	const float diff = fabs(a - b);
	if(diff > M_PI)
		return diff - M_PI;

	return diff;
}

float zerof(float f, float approach)
{
	if(f > 0.0f)
	{
		f -= approach;
		if(f < 0.0f)
			f = 0.0f;
	}
	else if(f < 0.0f)
	{
		f += approach;
		if(f > 0.0f)
			f = 0.0f;
	}

	return f;
}

}
