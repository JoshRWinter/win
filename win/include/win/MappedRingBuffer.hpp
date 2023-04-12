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
		slices[0].start = parent.original_head;
		slices[0].length = std::min(parent.parent.buffer_length - parent.original_head, parent.original_length);
		slices[1].start = 0;
		slices[1].length = parent.original_length - slices[0].length;
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
	explicit MappedRingBufferRangeIterator(MappedRingBufferRange<T> *parent)
		: parent(parent)
	{}

	void operator++() { move_next(); }
	void operator++(int) { move_next(); }

	T &operator*() { return parent->parent.buffer[parent->current_head]; }

	bool operator==(const MappedRingBufferRangeIterator<T> &rhs) const { return parent == rhs.parent; }
	bool operator!=(const MappedRingBufferRangeIterator<T> &rhs) const { return parent != rhs.parent; }

private:
	void move_next()
	{
		parent->current_head = (parent->current_head + 1) % parent->parent.buffer_length;
		--parent->current_remaining;

		// set parent to null if out of items.
		parent = parent->current_remaining != 0 ? parent : NULL;
	}

	MappedRingBufferRange<T> *parent; // this being null signifies that this iterator is the "end" iterator
};

template <typename T> class MappedRingBufferRange
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedRingBufferRangeIterator<T>;
	friend class MappedRingBufferRangeSliceIterable<T>;

public:
	MappedRingBufferRange(int head, int length, MappedRingBuffer<T> &parent)
		: original_head(head)
		, original_length(length)
		, current_head(head)
		, current_remaining(length)
		, parent(parent)
	{}

	MappedRingBufferRangeIterator<T> begin() { return MappedRingBufferRangeIterator<T>(current_remaining != 0 ? this : NULL); }
	MappedRingBufferRangeIterator<T> end() { return MappedRingBufferRangeIterator<T>(NULL); }

	MappedRingBufferRangeSliceIterable<T> slices() const { return MappedRingBufferRangeSliceIterable<T>(*this); }

	int head() const { return current_head; }
	int remaining() const { return current_remaining; }

	T &operator[](int index)
	{
#ifndef NDEBUG
		if (index < 0 || index >= original_length)
			win::bug("MappedBufferRange: index is out of bounds. Range length is " + std::to_string(original_length));
#endif

		const int real_index = (original_head + index) % parent.buffer_length;
		return parent.buffer[real_index];
	}

	int write(const T *const src, const int len)
	{
		if (len > current_remaining)
			win::bug("MappedBufferRange: write too long. Requested write length was " + std::to_string(len) + " while the range has only " + std::to_string(current_remaining) + " remaining.");

		const int available = parent.buffer_length - current_head;
		const int put = std::min(available, len);

		const int available2 = current_remaining - put;
		const int put2 = std::min(available2, len - put);

		const int put_bytes = put * sizeof(T);
		const int put2_bytes = put2 * sizeof(T);

		memcpy(parent.buffer + current_head, src, put_bytes);
		memcpy(parent.buffer, src + put, put2_bytes);

		const int advancement = (put + put2);
		current_head = (current_head + advancement) % parent.buffer_length;
		current_remaining -= advancement;

		return advancement;
	}

private:
	const int original_head;
	const int original_length;
	int current_head;
	int current_remaining;

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
		if (len > buffer_length)
			win::bug("MappedBuffer: reservation too long. Requested length was " + std::to_string(len) + " while the total buffer length is only " + std::to_string(buffer_length) + ".");

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
