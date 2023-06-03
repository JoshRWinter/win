#pragma once

#include <type_traits>
#include <cstring>

#include <win/Win.hpp>

namespace win
{

template <typename T> class MappedRingBuffer;
template <typename T, bool contiguous = false> class MappedRingBufferRange;
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
	explicit MappedRingBufferRangeSliceIterable(const MappedRingBufferRange<T, false> &parent)
	{
		slices[0].start = parent.range_head;
		slices[0].length = std::min(parent.buffer_length - parent.range_head, parent.range_length);
		slices[1].start = 0;
		slices[1].length = parent.range_length - slices[0].length;
	}

	MappedRingBufferRangeSliceIterator<T> begin() const { return MappedRingBufferRangeSliceIterator<T>(*this, 0); }
	MappedRingBufferRangeSliceIterator<T> end() const { return MappedRingBufferRangeSliceIterator<T>(*this, 1 + (slices[1].length != 0)); }

private:
	MappedRingBufferRangeSlice slices[2];
};

template <typename T, bool contiguous = false> class MappedRingBufferRangeIterator
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:
	MappedRingBufferRangeIterator(MappedRingBufferRange<T, contiguous> *parent, int position)
		: parent(parent)
		, position(position)
	{}

	MappedRingBufferRangeIterator<T, contiguous> operator+(int i) const { return MappedRingBufferRangeIterator<T, contiguous>(parent, position + i); }
	MappedRingBufferRangeIterator<T, contiguous> operator-(int i) const { return MappedRingBufferRangeIterator<T, contiguous>(parent, position - i); }

	MappedRingBufferRangeIterator<T, contiguous> operator++() { ++position; return MappedRingBufferRangeIterator<T, contiguous>(parent, position); }
	MappedRingBufferRangeIterator<T, contiguous> operator++(int) { ++position; return MappedRingBufferRangeIterator<T, contiguous>(parent, position - 1); }

	MappedRingBufferRangeIterator<T, contiguous> operator--() { --position; return MappedRingBufferRangeIterator<T, contiguous>(parent, position); }
	MappedRingBufferRangeIterator<T, contiguous> operator--(int) { --position; return MappedRingBufferRangeIterator<T, contiguous>(parent, position + 1); }

	bool operator==(const MappedRingBufferRangeIterator<T, contiguous> &rhs) const { return parent == rhs.parent && position == rhs.position; }
	bool operator!=(const MappedRingBufferRangeIterator<T, contiguous> &rhs) const { return parent != rhs.parent || position != rhs.position; }

	T &operator*()
	{
#ifndef NDEBUG
		if (position < 0 || position >= parent->range_length)
			win::bug("MappedRingBufferRangeIterator: Attempt to access invalid range index " + std::to_string(position) + " when range length is " + std::to_string(parent->range_length) + ".");
#endif

		if constexpr (contiguous)
			return parent->buffer[(parent->range_head + position)];
		else
			return parent->buffer[(parent->range_head + position) % parent->buffer_length];
	}

private:
	int position;
	MappedRingBufferRange<T, contiguous> *parent;
};

template <typename T, bool contiguous> class MappedRingBufferRange
{
	WIN_NO_COPY(MappedRingBufferRange);
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	friend class MappedRingBufferRangeIterator<T, contiguous>;
	friend class MappedRingBufferRangeSliceIterable<T>;

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

	MappedRingBufferRange(MappedRingBufferRange<T, contiguous> &&rhs) noexcept = default;

	MappedRingBufferRangeIterator<T, contiguous> begin() { return MappedRingBufferRangeIterator<T, contiguous>(this, 0); }
	MappedRingBufferRangeIterator<T, contiguous> end() { return MappedRingBufferRangeIterator<T, contiguous>(this, range_length); }

	MappedRingBufferRangeSliceIterable<T> slices() const
	{
		static_assert(!contiguous, "slices() not supported (or necessary) on contiguous ranges");
		return MappedRingBufferRangeSliceIterable<T>(*this);
	}

	int head() const { return range_head; }
	int length() const { return range_length; }

	MappedRingBufferRange<T, contiguous> &operator=(MappedRingBufferRange<T, contiguous> &&rhs) noexcept = default;

	T &operator[](int index)
	{
#ifndef NDEBUG
		if (index < 0 || index >= range_length)
			win::bug("MappedRingBufferRange: index is out of bounds. Range length is " + std::to_string(range_length));
#endif

		const int real_index = contiguous ? range_head + index : ((range_head + index) % buffer_length);
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

		if constexpr (!contiguous)
		{
			const int available2 = buffer_length - put;
			const int put2 = std::min(available2, len - put);
			const int put2_bytes = put2 * sizeof(T);

			memcpy(buffer, src + put, put2_bytes);
		}
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
	friend class MappedRingBufferRange<T>;
	friend class MappedRingBufferRange<T, true>;
	friend class MappedRingBufferRangeIterator<T>;
	friend class MappedRingBufferRangeIterator<T, true>;

public:
	MappedRingBuffer(void *mapbuf, int len_elelements)
		: buffer_head(0)
		, buffer(reinterpret_cast<T*>(mapbuf))
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

	MappedRingBufferRange<T, true> reserve_contiguous(int len)
	{
#ifndef NDEBUG
		if (len > buffer_length)
			win::bug("MappedRingBuffer: contiguous reservation too long. Requested length was " + std::to_string(len) + " while the total buffer length is only " + std::to_string(buffer_length) + ".");
#endif

		int start;
		if (buffer_length - buffer_head < len)
			start = 0;
		else
			start = buffer_head;

		MappedRingBufferRange<T, true> range(buffer, buffer_length, start, len);
		buffer_head = (start + len) % buffer_length;

		return range;
	}

private:
	int buffer_head;
	T *buffer;
	int buffer_length;
};

}
