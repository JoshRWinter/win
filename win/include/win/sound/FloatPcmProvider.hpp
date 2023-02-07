#pragma once

#include <win/Win.hpp>

namespace win
{

class FloatPcmProvider
{
	WIN_NO_COPY_MOVE(FloatPcmProvider);

public:
	static constexpr int default_working_buffer_size = 4096;

	virtual int read_samples(float *dest, int len) = 0;

protected:
	FloatPcmProvider() = default;
};

}
