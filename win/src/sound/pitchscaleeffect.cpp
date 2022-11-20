#include <win/sound/pitchscaleeffect.hpp>

namespace win
{

PitchScaleEffect::PitchScaleEffect(int priority, float scale)
	: SoundEffect(priority)
	, scale(scale)
{}

int PitchScaleEffect::read_samples(float *const dest, const int len)
{
	win::bug("this is unimplemented");
}

void PitchScaleEffect::set_scale(float s)
{
	scale = s;
}

}
