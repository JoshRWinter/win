#pragma once

#include <win/MappedRingBuffer.hpp>

namespace win
{

struct GLMappedRingBufferReservation
{
	GLMappedRingBufferReservation(int buffer_length, int start, int length)
		: buffer_length(buffer_length)
		, start(start)
		, length(length)
	{}

	bool conflicts(const GLMappedRingBufferReservation &rhs) const
	{
		return
			(start < rhs.start + rhs.length && start + length > rhs.start) ||
			(start + buffer_length < rhs.start + rhs.length && start + buffer_length + length > rhs.start) ||
			(start < rhs.start + buffer_length + rhs.length && start + length > rhs.start + buffer_length);
	}

	const int buffer_length;
	int start;
	int length;
};

template<typename T> class GLMappedRingBuffer
{
	WIN_NO_COPY_MOVE(GLMappedRingBuffer);

public:
	GLMappedRingBuffer(void *mem, int length_elements)
		: inner(mem, length_elements)
	{}

	MappedRingBufferRange<T> reserve(int len)
	{
		return inner.reserve(len);
	}

private:
	MappedRingBuffer<T> inner;
};

}
