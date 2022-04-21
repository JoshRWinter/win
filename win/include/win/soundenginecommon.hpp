#ifndef WIN_SOUNDENGNECOMMON_HPP
#define WIN_SOUNDENGNECOMMON_HPP

#include <cstdint>
#include <cstdlib>

namespace win
{

typedef void (*SoundConfigFn)(float, float, float, float, float*, float*);
inline constexpr int SOUNDENGINE_MAX_SOUNDS = 32;

inline void soundengine_channel_dupe(std::int16_t *const dest, const std::int16_t *const source, const size_t source_len)
{
	for(size_t i = 0; i < source_len * 2; i += 2)
	{
		dest[i] = source[i / 2];
		dest[i + 1] = source[i / 2];
	}
}

inline void default_sound_config_fn(float, float, float, float, float *volume, float *balance)
{
	*volume = 1.0f;
	*balance = 0.0f;
}

inline float soundengine_clamp_volume(float v)
{
	if(v > 1.0f)
		return 1.0f;
	else if(v < 0.0f)
		return 0.0f;

	return v;
}

inline float soundengine_clamp_balance(float bal)
{
	if(bal > 1.0f)
		return 1.0f;
	else if(bal < -1.0f)
		return -1.0f;

	return bal;
}

inline void soundengine_get_config(SoundConfigFn config_fn, float listenerx, float listenery, float sourcex, float sourcey, float *volume_l, float *volume_r)
{
	float volume = 0.0f, balance = 0.0f;

	config_fn(listenerx, listenery, sourcex, sourcey, &volume, &balance);

	// clamp [0.0, 1.0f]
	volume = soundengine_clamp_volume(volume);

	// clamp [-1.0f, 1.0f]
	balance = soundengine_clamp_balance(balance);

	// convert to volumes
	*volume_l = volume;
	*volume_r = volume;

	if(balance > 0.0f)
		*volume_l -= balance;
	else if(balance < 0.0f)
		*volume_r += balance;

	// reclamp
	*volume_l = soundengine_clamp_volume(*volume_l);
	*volume_r = soundengine_clamp_volume(*volume_r);
}

}

#endif
