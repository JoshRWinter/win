#include <win/sound/pitchscaleeffect.hpp>

namespace win
{

PitchScaleEffect::PitchScaleEffect(int priority, float scale)
	: SoundEffect(priority)
	, scale(scale)
{}

int PitchScaleEffect::read_samples(float *const dest, const int len)
{
	if (inner == NULL)
		win::bug("SoundEffect: null inner");
	if (len / 2 > default_working_buffer_size)
		win::bug("PitchScaleEffect: request len too big");

	float convbuf[default_working_buffer_size];
	const int got = inner->read_samples(convbuf, len / 2);

	for (int i = 0; i < len; ++i)
		dest[i] = convbuf[i / 2];

	return got * 2;
}

void PitchScaleEffect::set_scale(float s)
{
	scale = s;
}

}
