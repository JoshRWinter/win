#pragma once

#include <type_traits>
#include <cstring>

#include <win/Win.hpp>

namespace win
{

template <typename T> class MappedRingBufferRange;

template <typename T> class MappedRingBufferRangeIterator
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	MappedRingBufferRangeIterator(MappedRingBufferRange<T> *parent, int position)
		: parent(parent)
		, position(position)
	{}

	MappedRingBufferRangeIterator<T> operator+(int i) const { return MappedRingBufferRangeIterator<T>(parent, position + i); }
	MappedRingBufferRangeIterator<T> operator-(int i) const { return MappedRingBufferRangeIterator<T>(parent, position - i); }

	MappedRingBufferRangeIterator<T> operator++() { ++position; return MappedRingBufferRangeIterator<T>(parent, position); }
	MappedRingBufferRangeIterator<T> operator++(int) { ++position; return MappedRingBufferRangeIterator<T>(parent, position - 1); }

	MappedRingBufferRangeIterator<T> operator--() { --position; return MappedRingBufferRangeIterator<T>(parent, position); }
	MappedRingBufferRangeIterator<T> operator--(int) { --position; return MappedRingBufferRangeIterator<T>(parent, position + 1); }

	bool operator==(const MappedRingBufferRangeIterator<T> &rhs) const { return parent == rhs.parent && position == rhs.position; }
	bool operator!=(const MappedRingBufferRangeIterator<T> &rhs) const { return parent != rhs.parent || position != rhs.position; }

	T &operator*()
	{
#ifndef NDEBUG
		if (position < 0 || position >= parent->range_length)
			win::bug("MappedRingBufferRangeIterator: Attempt to access invalid range index " + std::to_string(position) + " when range length is " + std::to_string(parent->range_length) + ".");
#endif

		return parent->buffer[(parent->range_head + position) % parent->buffer_length];
	}

private:
	MappedRingBufferRange<T> *parent;
	int position;
};

template <typename T> class MappedRingBufferRange
{
	WIN_NO_COPY(MappedRingBufferRange);
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedRingBufferRangeIterator<T>;

public:
	MappedRingBufferRange()
		: buffer(NULL)
		, buffer_length(-1)
		, range_head(-1)
		, range_length(-1)
	{}

	MappedRingBufferRange(T *buffer, int buffer_length, int head, int length)
		: buffer(buffer)
		, buffer_length(buffer_length)
		, range_head(head)
		, range_length(length)
	{}

	MappedRingBufferRange(MappedRingBufferRange<T> &&rhs) noexcept = default;
	MappedRingBufferRange<T> &operator=(MappedRingBufferRange<T> &&rhs) noexcept = default;

	MappedRingBufferRangeIterator<T> begin() { return MappedRingBufferRangeIterator<T>(this, 0); }
	MappedRingBufferRangeIterator<T> end() { return MappedRingBufferRangeIterator<T>(this, range_length); }

	int head() const { return range_head; }
	int length() const { return range_length; }

	T &operator[](int index)
	{
#ifndef NDEBUG
		if (index < 0 || index >= range_length)
			win::bug("MappedRingBufferRange: index is out of bounds. Range length is " + std::to_string(range_length));
#endif

		const int real_index = (range_head + index) % buffer_length;
		return buffer[real_index];
	}

	void write(const T *const src, const int len) { write(0, src, len); }

	void write(const int offset, const T *const src, const int len)
	{
#ifndef NDEBUG
		if (len + offset > range_length)
			win::bug("MappedRingBufferRange: write too long. Requested write length was " + std::to_string(len) + " (from offset " + std::to_string(offset) + ") while the range length is only " + std::to_string(range_length) + ".");
#endif

		const int adjusted_head = (range_head + offset) % buffer_length;

		const int available = buffer_length - adjusted_head;
		const int put = std::min(available, len);
		const int put_bytes = put * sizeof(T);

		memcpy(buffer + adjusted_head, src, put_bytes);

		const int available2 = buffer_length - put;
		const int put2 = std::min(available2, len - put);
		const int put2_bytes = put2 * sizeof(T);

		memcpy(buffer, src + put, put2_bytes);
	}

private:
	T *buffer;
	int buffer_length;

	int range_head;
	int range_length;
};

template <typename T> class MappedRingBuffer
{
	WIN_NO_COPY(MappedRingBuffer);

	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	MappedRingBuffer(void *mapbuf, int len_elelements)
		: buffer(reinterpret_cast<T*>(mapbuf))
		, buffer_head(0)
		, buffer_length(len_elelements)
	{}

	MappedRingBuffer(MappedRingBuffer &&rhs) noexcept = default;

	int head() const { return buffer_head; }
	int length() const { return buffer_length; }

	MappedRingBuffer &operator=(MappedRingBuffer &&rhs) noexcept = default;

	MappedRingBufferRange<T> reserve(int len)
	{
#ifndef NDEBUG
		if (len > buffer_length)
			win::bug("MappedRingBuffer: reservation too long. Requested length was " + std::to_string(len) + " while the total buffer length is only " + std::to_string(buffer_length) + ".");
#endif

		MappedRingBufferRange<T> range(buffer, buffer_length, buffer_head, len);

		buffer_head = (buffer_head + len) % buffer_length;

		return range;
	}

private:
	T *buffer;
	int buffer_head;
	int buffer_length;
};

}
