#pragma once

#include <type_traits>
#include <atomic>
#include <algorithm>

#include <string.h>

#include <win/win.hpp>

namespace win
{

class RingBufferBase
{
public:
	template <typename T> static int read(const T *const ringbuffer, const int ringbuffer_length, T *const dest, int len, int &read_cursor, const int write_cursor)
	{
		static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

		if (read_cursor > write_cursor)
		{
			const int available = ringbuffer_length - read_cursor;
			const int take = std::min(available, len);
	    	memcpy(dest, ringbuffer + read_cursor, take * sizeof(T));
			read_cursor = (read_cursor + take) % ringbuffer_length;

			// handle wrap case
			if (read_cursor == 0)
				return take + read(ringbuffer, ringbuffer_length, dest + take, len - take, read_cursor, write_cursor);
			else
				return take;
		}
		else
		{
	    	const int available = write_cursor - read_cursor;
			const int take = std::min(available, len);
			memcpy(dest, ringbuffer + read_cursor, take * sizeof(T));
			read_cursor += take;

			return take;
		}
	}

	template <typename T> static int write(T *const ringbuffer, const int ringbuffer_length, const T *const src, int len, const int read_cursor, int &write_cursor)
	{
		static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

		if (read_cursor > write_cursor)
		{
			const int available = (read_cursor - write_cursor) - 1;
			const int put = std::min(available, len);
			memcpy(ringbuffer + write_cursor, src, put * sizeof(T));
			write_cursor += put;

			return put;
		}
		else
		{
			const int available = ringbuffer_length - write_cursor - (read_cursor == 0 ? 1 : 0);
			const int put = std::min(available, len);
			memcpy(ringbuffer + write_cursor, src, put * sizeof(T));
			write_cursor = (write_cursor + put) % ringbuffer_length;

			// handle wrap case
			if (write_cursor == 0 && len - put > 0)
				return put + write(ringbuffer, ringbuffer_length, src + put, len - put, read_cursor, write_cursor);
			else
				return put;
		}
	}

	template <typename T> static int size(const int ringbuffer_length, const int read_cursor, const int write_cursor)
	{
	    if (read_cursor > write_cursor)
			return (ringbuffer_length - read_cursor) + write_cursor;
		else
			return write_cursor - read_cursor;
	}
};

template <typename T, int length> class ConcurrentRingBuffer
{
    WIN_NO_COPY_MOVE(ConcurrentRingBuffer);
public:
	ConcurrentRingBuffer()
	{
		read_cursor.store(0);
		write_cursor.store(0);
	}

    int read(T *const dest, int len)
	{
	    int rc = read_cursor.load();
		const int wc = write_cursor.load();

		const int got = RingBufferBase::read(ringbuffer, length, dest, len, rc, wc);

		read_cursor.store(rc);

		return got;
	}

	int write(const T *const src, int len)
	{
		const int rc = read_cursor.load();
		int wc = write_cursor.load();

		const int put = RingBufferBase::write(ringbuffer, length, src, len, rc, wc);

		write_cursor.store(wc);

		return put;
	}

	int size() const
	{
		const int rc = read_cursor.load();
		const int wc = write_cursor.load();

		return RingBufferBase::size<T>(length, rc, wc);
	}

private:
	std::atomic<int> read_cursor;
	std::atomic<int> write_cursor;
	T ringbuffer[length];
};

}
