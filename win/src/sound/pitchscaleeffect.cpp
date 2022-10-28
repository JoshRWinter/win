#include <win/sound/pitchscaleeffect.hpp>

namespace win
{

PitchScaleEffect::PitchScaleEffect(int scale)
	: scale(scale)
{}

int PitchScaleEffect::apply(int (*read)(int))
{
	return 0;
}

void PitchScaleEffect::set_scale(int s)
{
	scale = s;
}

}