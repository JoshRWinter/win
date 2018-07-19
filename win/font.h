#ifndef WIN_FONT_H
#define WIN_FONT_H

#include <array>

namespace win
{

class font
{
	friend display;

	struct kernvector { float advance, bitmap_left; };

public:
	font(const font&) = delete;
	font(font&&);
	~font();

private:
	font(display&, resource&, float, int, int, float, float);

	unsigned atlas_;
	std::array<kernvector, 96> kern_;
	display &parent_;
	float size_;
};

}

#endif
