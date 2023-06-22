#pragma once

#include <win/sound/PcmSource.hpp>

namespace win
{

class CachedPcmSource : PcmSource
{
	WIN_NO_COPY_MOVE(CachedPcmSource);

public:
	CachedPcmSource(int channel_count, const float *data, long length);

	int channels() override;
	bool empty() override;
	void restart() override;
	int read_samples(float *buf, int read_samples) override;

private:
	int channel_count;
	long bookmark;
	const float *pcmdata;
	long pcmdata_length;
};

}
