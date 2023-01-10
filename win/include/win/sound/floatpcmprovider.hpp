#pragma once

#include <win/win.hpp>

namespace win
{

class FloatPCMProvider
{
	WIN_NO_COPY_MOVE(FloatPCMProvider);

public:
	static constexpr int default_working_buffer_size = 4096;

	virtual int read_samples(float *dest, int len) = 0;

protected:
	FloatPCMProvider() = default;
};

}
