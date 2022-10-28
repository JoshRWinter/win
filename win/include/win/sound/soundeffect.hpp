#ifndef WIN_SOUND_EFFECT_HPP
#define WIN_SOUND_EFFECT_HPP

namespace win
{

class SoundEffect
{
public:
	virtual int apply(int (*readdata)(int)) = 0;
	virtual ~SoundEffect() = default;
};

}

#endif