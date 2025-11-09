#pragma once

#include <type_traits>
#include <atomic>

#include <cstring>

#include <win/Win.hpp>

namespace win
{

template <typename T, int desired_length = -1> class ConcurrentRingBuffer
{
	WIN_NO_COPY_MOVE(ConcurrentRingBuffer);

	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

public:

	constexpr static auto length = desired_length;

	ConcurrentRingBuffer(T *user_buffer, int user_length)
		: atomic_read_cursor(0)
		, atomic_write_cursor(0)
		, ringbuffer(user_buffer)
		, ringbuffer_length(user_length)
	{
		if (user_length < 2)
			win::bug("ConcurrentRingBuffer: length must be at least 2");
	}

	ConcurrentRingBuffer()
		: atomic_read_cursor(0)
		, atomic_write_cursor(0)
		, ringbuffer(default_storage)
		, ringbuffer_length(desired_length)
	{
		static_assert(desired_length > 1, "desired_length must be at least 2");
	}

	int read(T *const dest, const int len)
	{
		const int read_cursor = atomic_read_cursor.load();
		const int write_cursor = atomic_write_cursor.load();

		int take;
		int take2;

		if (read_cursor > write_cursor)
		{
			const int available = ringbuffer_length - read_cursor;
			take = std::min(available, len);

			const int available2 = write_cursor;
			take2 = std::min(available2, len - take);
		}
		else
		{
	    	const int available = write_cursor - read_cursor;
			take = std::min(available, len);

			// no wrapping here
			take2 = 0;
		}

		const int take_bytes = take * sizeof(T);
		const int take2_bytes = take2 * sizeof(T);

		memcpy(dest, ringbuffer + read_cursor, take_bytes);
		memcpy(dest + take, ringbuffer, take2_bytes);

		atomic_read_cursor.store((read_cursor + take + take2) % ringbuffer_length);

		return take + take2;
	}

	int write(const T *const src, const int len)
	{
		const int read_cursor = atomic_read_cursor.load();
		const int write_cursor = atomic_write_cursor.load();

		int put;
		int put2;

		if (read_cursor > write_cursor)
		{
			const int writeable = (read_cursor - write_cursor) - 1;
			put = std::min(writeable, len);

			// no wrapping here
			put2 = 0;
		}
		else
		{
			const bool zero_read = read_cursor == 0;

			const int writeable = (ringbuffer_length - write_cursor) - (zero_read ? 1 : 0);
			put = std::min(writeable, len);

			const int writeable2 = zero_read ? 0 : (read_cursor - 1);
			put2 = std::min(writeable2, len - put);
		}

		const int put_bytes = put * sizeof(T);
		const int put2_bytes = put2 * sizeof(T);

		memcpy(ringbuffer + write_cursor, src, put_bytes);
		memcpy(ringbuffer, src + put, put2_bytes);

		atomic_write_cursor.store((write_cursor + put + put2) % ringbuffer_length);

		return put + put2;
	}

	int size() const
	{
		const int read_cursor = atomic_read_cursor.load();
		const int write_cursor = atomic_write_cursor.load();

		if (read_cursor > write_cursor)
			return (ringbuffer_length - read_cursor) + write_cursor;
		else
			return write_cursor - read_cursor;
	}

private:
	std::atomic<int> atomic_read_cursor;
	std::atomic<int> atomic_write_cursor;

	T *ringbuffer;
	int ringbuffer_length;

	T default_storage[desired_length > 0 ? desired_length : 0];
};

}
