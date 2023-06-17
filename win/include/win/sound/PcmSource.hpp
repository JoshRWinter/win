#pragma once

#include <cstdint>

namespace win
{

class PcmSource
{
public:
	virtual int channels() = 0;
	virtual bool empty() = 0;
	virtual void restart() = 0;
	virtual int read_samples(float *buf, int read_samples) = 0;

protected:
	PcmSource() = default;
};

}
