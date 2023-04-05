#pragma once

#include <type_traits>
#include <cstring>

#include <win/Win.hpp>

namespace win
{

template <typename T> class MappedBufferRange;
template <typename T> class MappedBufferRangeIterator
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	explicit MappedBufferRangeIterator(MappedBufferRange<T> *parent)
		: parent(parent)
	{}

	void operator=(const MappedBufferRangeIterator<T>&) = delete;
	void operator=(MappedBufferRangeIterator<T>&&) = delete;

	void operator++() { move_next(); }
	void operator++(int) { move_next(); }

	T &operator*() { return parent->parent.buffer[parent->head]; }

	bool operator==(const MappedBufferRangeIterator<T> &rhs) const { return parent == rhs.parent; }
	bool operator!=(const MappedBufferRangeIterator<T> &rhs) const { return parent != rhs.parent; }

private:
	void move_next()
	{
		parent->head = (parent->head + 1) % parent->parent.buffer_length;
		--parent->remaining;

		// set parent to null if out of items.
		parent = parent->remaining != 0 ? parent : NULL;
	}

	MappedBufferRange<T> *parent; // this being null signifies that this iterator is the "end" iterator
};

template <typename T> class MappedBuffer;
template <typename T> class MappedBufferRange
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	MappedBufferRange(int head, int length, MappedBuffer<T> &parent)
		: original_head(head)
		, original_length(length)
		, head(head)
		, remaining(length)
		, parent(parent)
	{}

	MappedBufferRangeIterator<T> begin() { return MappedBufferRangeIterator<T>(remaining != 0 ? this : NULL); }
	MappedBufferRangeIterator<T> end() { return MappedBufferRangeIterator<T>(NULL); }

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
		if (len > remaining)
			win::bug("MappedBufferRange: write too long. Requested write length was " + std::to_string(len) + " while the range has only " + std::to_string(remaining) + " remaining.");

		const int available = parent.buffer_length - head;
		const int put = std::min(available, len);

		const int available2 = remaining - put;
		const int put2 = std::min(available2, len - put);

		const int put_bytes = put * sizeof(T);
		const int put2_bytes = put2 * sizeof(T);

		memcpy(parent.buffer + head, src, put_bytes);
		memcpy(parent.buffer, src + put, put2_bytes);

		const int advancement = (put + put2);
		head = (head + advancement) % parent.buffer_length;
		remaining -= advancement;

		return advancement;
	}

private:
	const int original_head;
	const int original_length;
	int head;
	int remaining;

	MappedBuffer<T> &parent;
};

template <typename T> class MappedBuffer
{
	WIN_NO_COPY_MOVE(MappedBuffer);
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedBufferRange<T>;
	friend class MappedBufferRangeIterator<T>;

public:
	MappedBuffer(void *mapbuf, int len_elelements)
		: head(0)
		, buffer(reinterpret_cast<T*>(mapbuf))
		, buffer_length(len_elelements)
	{}

	MappedBufferRange<T> reserve(int len)
	{
		if (len > buffer_length)
			win::bug("MappedBuffer: reservation too long. Requested length was " + std::to_string(len) + " while the total buffer length is only " + std::to_string(buffer_length) + ".");

		MappedBufferRange<T> range(head, len, *this);

		head = (head + len) % buffer_length;

		return range;
	}

private:
	int head;
	T *const buffer;
	const int buffer_length;
};

}
