#pragma once

#include <type_traits>
#include <cstring>

#include <win/Win.hpp>

namespace win
{

template <typename T> class MappedRingBuffer;
template <typename T> class MappedRingBufferRange;
template <typename T> class MappedRingBufferRangeSliceIterable;

struct MappedRingBufferRangeSlice
{
	int start;
	int length;
};

template <typename T> class MappedRingBufferRangeSliceIterator
{
public:
	MappedRingBufferRangeSliceIterator(const MappedRingBufferRangeSliceIterable<T> &parent, int position)
		: parent(parent)
		, position(position)
	{}

	void operator++() { ++position; }
	void operator++(int) { ++position; }

	const MappedRingBufferRangeSlice &operator*() const { return parent.slices[position]; }

	bool operator==(const MappedRingBufferRangeSliceIterator<T> &rhs) const { return &parent == &rhs.parent && position == rhs.position; }
	bool operator!=(const MappedRingBufferRangeSliceIterator<T> &rhs) const { return &parent != &rhs.parent || position != rhs.position; }

private:
	const MappedRingBufferRangeSliceIterable<T> &parent;
	int position;
};

template <typename T> class MappedRingBufferRangeSliceIterable
{
	friend class MappedRingBufferRangeSliceIterator<T>;

public:
	explicit MappedRingBufferRangeSliceIterable(const MappedRingBufferRange<T> &parent)
	{
		slices[0].start = parent.range_head;
		slices[0].length = std::min(parent.parent.buffer_length - parent.range_head, parent.range_length);
		slices[1].start = 0;
		slices[1].length = parent.range_length - slices[0].length;
	}

	MappedRingBufferRangeSliceIterator<T> begin() const { return MappedRingBufferRangeSliceIterator<T>(*this, 0); }
	MappedRingBufferRangeSliceIterator<T> end() const { return MappedRingBufferRangeSliceIterator<T>(*this, 1 + (slices[1].length != 0)); }

private:
	MappedRingBufferRangeSlice slices[2];
};

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

		return parent->parent.buffer[(parent->range_head + position) % parent->parent.buffer_length];
	}

private:
	int position;
	MappedRingBufferRange<T> *parent;
};

template <typename T> class MappedRingBufferRange
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedRingBufferRangeIterator<T>;
	friend class MappedRingBufferRangeSliceIterable<T>;

public:
	MappedRingBufferRange(int head, int length, MappedRingBuffer<T> &parent)
		: range_head(head)
		, range_length(length)
		, parent(parent)
	{}

	MappedRingBufferRangeIterator<T> begin() { return MappedRingBufferRangeIterator<T>(this, 0); }
	MappedRingBufferRangeIterator<T> end() { return MappedRingBufferRangeIterator<T>(this, range_length); }

	MappedRingBufferRangeSliceIterable<T> slices() const { return MappedRingBufferRangeSliceIterable<T>(*this); }

	int head() const { return range_head; }
	int length() const { return range_length; }

	T &operator[](int index)
	{
#ifndef NDEBUG
		if (index < 0 || index >= range_length)
			win::bug("MappedBufferRange: index is out of bounds. Range length is " + std::to_string(range_length));
#endif

		const int real_index = (range_head + index) % parent.buffer_length;
		return parent.buffer[real_index];
	}

	int write(const T *const src, const int len)
	{
		return write(0, src, len);
	}

	int write(const int offset, const T *const src, const int len)
	{
#ifndef NDEBUG
		if (len > range_length)
			win::bug("MappedBufferRange: write too long. Requested write length was " + std::to_string(len) + " while the range length is only " + std::to_string(range_length) + ".");
#endif

		const int adjusted_head = (range_head + offset) % parent.buffer_length;

		const int available = parent.buffer_length - adjusted_head;
		const int put = std::min(available, len);

		const int available2 = parent.buffer_length - put;
		const int put2 = std::min(available2, len - put);

		const int put_bytes = put * sizeof(T);
		const int put2_bytes = put2 * sizeof(T);

		memcpy(parent.buffer + adjusted_head, src, put_bytes);
		memcpy(parent.buffer, src + put, put2_bytes);

		return put + put2;
	}

private:
	const int range_head;
	const int range_length;

	MappedRingBuffer<T> &parent;
};

template <typename T> class MappedRingBuffer
{
	WIN_NO_COPY_MOVE(MappedRingBuffer);
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedRingBufferRange<T>;
	friend class MappedRingBufferRangeIterator<T>;

public:
	MappedRingBuffer(void *mapbuf, int len_elelements)
		: buffer_head(0)
		, buffer(reinterpret_cast<T*>(mapbuf))
		, buffer_length(len_elelements)
	{}

	int head() const { return buffer_head; }
	int length() const { return buffer_length; }

	MappedRingBufferRange<T> reserve(int len)
	{
#ifndef NDEBUG
		if (len > buffer_length)
			win::bug("MappedRingBuffer: reservation too long. Requested length was " + std::to_string(len) + " while the total buffer length is only " + std::to_string(buffer_length) + ".");
#endif

		MappedRingBufferRange<T> range(buffer_head, len, *this);

		buffer_head = (buffer_head + len) % buffer_length;

		return range;
	}

private:
	int buffer_head;
	T *const buffer;
	const int buffer_length;
};

}
