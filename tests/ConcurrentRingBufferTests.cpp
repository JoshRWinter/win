#include <win/Win.hpp>

#define private public
#include <win/ConcurrentRingBuffer.hpp>

static int successfull = 0;
#define assert(expression) do { if (!(expression)) { win::bug("Assert failed on line " + std::to_string(__LINE__)); } else { ++successfull; } } while (false)

template <typename T> void test()
{
	T ringbuf[5] = { 0, 0, 0, 0, 0 };

	int read;
	int wrote;

	win::ConcurrentRingBuffer<T> rb(ringbuf, 5);

	assert(rb.size() == 0);

	assert(rb.atomic_read_cursor.load() == 0);
	assert(rb.atomic_write_cursor.load() == 0);

	/////////////////////////////////////////////////////
	{
		const T src[] = { 12, 68, 44, 120 };
		wrote = rb.write(src, 100);

		assert(rb.size() == 4);
		assert(wrote == 4);
		assert(ringbuf[0] == 12 && ringbuf[1] == 68 && ringbuf[2] == 44 && ringbuf[3] == 120 && ringbuf[4] == 0);

		assert(rb.atomic_read_cursor.load() == 0);
		assert(rb.atomic_write_cursor.load() == 4);
	}
	/////////////////////////////////////////////////////
	{
		T dest[] = { 0 };
		read = rb.read(dest, 1);

		assert(rb.size() == 3);
		assert(read == 1);
		assert(dest[0] == 12);

		assert(rb.atomic_read_cursor.load() == 1);
		assert(rb.atomic_write_cursor.load() == 4);
	}
	/////////////////////////////////////////////////////
	{
		const T src[] = { 85 };
		wrote = rb.write(src, 100);

		assert(rb.size() == 4);
		assert(wrote == 1);
		assert(ringbuf[0] == 12 && ringbuf[1] == 68 && ringbuf[2] == 44 && ringbuf[3] == 120 && ringbuf[4] == 85);

		assert(rb.atomic_read_cursor.load() == 1);
		assert(rb.atomic_write_cursor.load() == 0);
	}
	/////////////////////////////////////////////////////
	{
		T dest[] = { 0, 0 };
		read = rb.read(dest, 2);

		assert(rb.size() == 2);
		assert(read == 2);
		assert(dest[0] == 68 && dest[1] == 44);

		assert(rb.atomic_read_cursor.load() == 3);
		assert(rb.atomic_write_cursor.load() == 0);
	}
	/////////////////////////////////////////////////////
	{
		const T src[] = { 2, 69 };
		wrote = rb.write(src, 100);

		assert(rb.size() == 4);
		assert(wrote == 2);
		assert(ringbuf[0] == 2 && ringbuf[1] == 69 && ringbuf[2] == 44 && ringbuf[3] == 120 && ringbuf[4] == 85);

		assert(rb.atomic_read_cursor.load() == 3);
		assert(rb.atomic_write_cursor.load() == 2);
	}
	/////////////////////////////////////////////////////
	{
		T dest[] = { 0, 0, 0 };
		read = rb.read(dest, 3);

		assert(rb.size() == 1);
		assert(read == 3);
		assert(dest[0] == 120 && dest[1] == 85 && dest[2] == 2);

		assert(rb.atomic_read_cursor.load() == 1);
		assert(rb.atomic_write_cursor.load() == 2);
	}
	/////////////////////////////////////////////////////
	{
		const T src[] = { 101 };
		wrote = rb.write(src, 1);

		assert(rb.size() == 2);
		assert(wrote == 1);
		assert(ringbuf[0] == 2 && ringbuf[1] == 69 && ringbuf[2] == 101 && ringbuf[3] == 120 && ringbuf[4] == 85);

		assert(rb.atomic_read_cursor.load() == 1);
		assert(rb.atomic_write_cursor.load() == 3);
	}
	/////////////////////////////////////////////////////
	{
		T dest[] = { 0, 0 };
		read = rb.read(dest, 100);

		assert(rb.size() == 0);
		assert(read == 2);
		assert(dest[0] == 69 && dest[1] == 101);

		assert(rb.atomic_read_cursor.load() == 3);
		assert(rb.atomic_write_cursor.load() == 3);
	}
	/////////////////////////////////////////////////////
	{
		const T src[] = { 55, 48, 63, 45 };
		wrote = rb.write(src, 100);

		assert(rb.size() == 4);
		assert(wrote == 4);
		assert(ringbuf[0] == 63 && ringbuf[1] == 45 && ringbuf[2] == 101 && ringbuf[3] == 55 && ringbuf[4] == 48);

		assert(rb.atomic_read_cursor.load() == 3);
		assert(rb.atomic_write_cursor.load() == 2);
	}
	/////////////////////////////////////////////////////
	{
		T dest[] = { 0, 0, 0, 0 };
		read = rb.read(dest, 100);

		assert(rb.size() == 0);
		assert(read == 4);
		assert(dest[0] == 55 && dest[1] == 48 && dest[2] == 63 && dest[3] == 45);

		assert(rb.atomic_read_cursor.load() == 2);
		assert(rb.atomic_write_cursor.load() == 2);
	}
}

int main()
{
	test<unsigned char>();
	test<unsigned long long>();
	test<float>();
	test<double>();

	fprintf(stderr, "all %d tests ran successfully\n", successfull);
}
