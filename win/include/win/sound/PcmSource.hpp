#pragma once

#include <cstdint>

#include <win/Win.hpp>

namespace win
{

class PcmSource
{
	WIN_NO_COPY_MOVE(PcmSource);

public:
	virtual int channels() = 0;
	virtual bool empty() = 0;
	virtual void restart() = 0;
	virtual int read_samples(float *buf, int samples) = 0;

protected:
	PcmSource() = default;
};

}
