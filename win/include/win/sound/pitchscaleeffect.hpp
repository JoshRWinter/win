#ifndef WIN_PITCH_SCALE_EFFECT_HPP
#define WIN_PITCH_SCALE_EFFECT_HPP

#include <win/sound/soundeffect.hpp>

namespace win
{

class PitchScaleEffect : public SoundEffect
{
public:
	PitchScaleEffect(int);

	int apply(int (*read)(int)) override;
	void set_scale(int);

private:
	int scale;
};

}

#endif
