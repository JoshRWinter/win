#include <stdio.h>

#include <win/ringbuffer.hpp>

static int testcount = 0;
#define rbassert(exp) if (!(exp)) { win::bug("Assert failed on line " + std::to_string(__LINE__)); } else { ++testcount; }

template <typename T> void run()
{
	const int canary = 127;
	int read;
	int write;

	T ringbuf_mem[6];
	T *ringbuf = ringbuf_mem + 1;

	T dest_mem[6];
	T *dest = dest_mem + 1;

	auto reset = [&]()
	{
		read = 0;
		write = 0;
		for (int i = 0; i < 6; ++i)
		{
			ringbuf_mem[i] = canary;
			dest_mem[i] = canary;
		}

		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0)
	};

	auto memcheck = [&]()
	{
		rbassert(ringbuf_mem[0] == canary);
		rbassert(ringbuf_mem[5] == canary);

		rbassert(dest_mem[0] == canary);
		rbassert(dest_mem[5] == canary);

		const int s = win::RingBufferBase::size<T>(4, read, write);
		rbassert(s >= 0 && s <= 4);
	};

	// make sure no read from empty buffer
	{
		reset();
		int got = win::RingBufferBase::read(ringbuf, 4, (T*)NULL, 1, read, write);
		rbassert(read == 0);
	}

	// make sure no override / overread
	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 4, read, write);
		rbassert(wrote == 3);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 0);
		rbassert(write == 3);
		rbassert(ringbuf[0] = 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 4, read, write);
		rbassert(got == 3);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 3);
		rbassert(write == 3);
		rbassert(dest[0] = 1 && dest[1] == 2 && dest[2] == 3);
		memcheck();
	}

	// general comprehensive tests
	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 2, read, write);
		rbassert(wrote == 2);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 0);
		rbassert(write == 2);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == canary && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 1);
		rbassert(write == 2);
		rbassert(dest[0] == 1 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// read
		const int got2 = win::RingBufferBase::read(ringbuf, 4, dest + 1, 3, read, write);
		rbassert(got2 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 2);
		rbassert(write == 2);
		rbassert(dest[0] == 1 && dest[1] == 2 && dest[2] == canary && dest[4] == canary);
		memcheck();
	}

	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 3, read, write);
		rbassert(wrote == 3);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 0);
		rbassert(write == 3);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 0, read, write);
		rbassert(got == 0);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 0);
		rbassert(write == 3);
		rbassert(dest[0] == canary && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// read
		const int got2 = win::RingBufferBase::read(ringbuf, 4, dest, 2, read, write);
		rbassert(got2 == 2);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 2);
		rbassert(write == 3);
		rbassert(dest[0] == 1 && dest[1] == 2 && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote2 = win::RingBufferBase::write(ringbuf, 4, src, 3, read, write);
		rbassert(wrote2 == 2);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 2);
		rbassert(write == 1);
		rbassert(ringbuf[0] == 2 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == 1);
		memcheck();
	}

	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 2, read, write);
		rbassert(wrote == 2);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 0);
		rbassert(write == 2);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == canary && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 1);
		rbassert(write == 2);
		rbassert(dest[0] == 1 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote2 = win::RingBufferBase::write(ringbuf, 4, src, 4, read, write);
		rbassert(wrote2 == 2);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 1);
		rbassert(write == 0);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 1 && ringbuf[3] == 2);
		memcheck();

		// read
		const int got2 = win::RingBufferBase::read(ringbuf, 4, dest, 10, read, write);
		rbassert(got2 == 3);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 0);
		rbassert(write == 0);
		rbassert(dest[0] == 2 && dest[1] == 1 && dest[2] == 2 && dest[4] == canary);
		memcheck();
	}

	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 100, read, write);
		rbassert(wrote == 3);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 0);
		rbassert(write == 3);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 1);
		rbassert(write == 3);
		rbassert(dest[0] == 1 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote2 = win::RingBufferBase::write(ringbuf, 4, src, 100, read, write);
		rbassert(wrote2 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 1);
		rbassert(write == 0);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == 1);
		memcheck();

		// read
		const int got2 = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got2 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 2);
		rbassert(write == 0);
		rbassert(dest[0] == 2 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote3 = win::RingBufferBase::write(ringbuf, 4, src, 100, read, write);
		rbassert(wrote3 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 2);
		rbassert(write == 1);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == 1);
		memcheck();

		// read
		const int got3 = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got3 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 3);
		rbassert(write == 1);
		rbassert(dest[0] == 3 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote4 = win::RingBufferBase::write(ringbuf, 4, src, 100, read, write);
		rbassert(wrote4 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 3);
		rbassert(read == 3);
		rbassert(write == 2);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 1 && ringbuf[2] == 3 && ringbuf[3] == 1);
		memcheck();

		// read
		const int got5 = win::RingBufferBase::read(ringbuf, 4, dest, 1, read, write);
		rbassert(got5 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 2);
		rbassert(read == 0);
		rbassert(write == 2);
		rbassert(dest[0] == 1 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();
	}

	{
		reset();
		T src[] = { 1, 2, 3, 4 };

		// write
		const int wrote = win::RingBufferBase::write(ringbuf, 4, src, 1, read, write);
		rbassert(wrote == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 0);
		rbassert(write == 1);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == canary && ringbuf[2] == canary && ringbuf[3] == canary);
		memcheck();

		// read
		const int got = win::RingBufferBase::read(ringbuf, 4, dest, 100, read, write);
		rbassert(got == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 1);
		rbassert(write == 1);
		rbassert(dest[0] == 1 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote2 = win::RingBufferBase::write(ringbuf, 4, src + 1, 1, read, write);
		rbassert(wrote2 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 1);
		rbassert(write == 2);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == canary && ringbuf[3] == canary);
		memcheck();

		// read
		const int got2 = win::RingBufferBase::read(ringbuf, 4, dest, 100, read, write);
		rbassert(got2 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 2);
		rbassert(write == 2);
		rbassert(dest[0] == 2 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote3 = win::RingBufferBase::write(ringbuf, 4, src + 2, 1, read, write);
		rbassert(wrote3 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 2);
		rbassert(write == 3);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == canary);
		memcheck();

		// read
		const int got3 = win::RingBufferBase::read(ringbuf, 4, dest, 100, read, write);
		rbassert(got3 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 3);
		rbassert(write == 3);
		rbassert(dest[0] == 3 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();

		// write
		const int wrote4 = win::RingBufferBase::write(ringbuf, 4, src + 3, 1, read, write);
		rbassert(wrote3 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 1);
		rbassert(read == 3);
		rbassert(write == 0);
		rbassert(ringbuf[0] == 1 && ringbuf[1] == 2 && ringbuf[2] == 3 && ringbuf[3] == 4);
		memcheck();

		// read
		const int got4 = win::RingBufferBase::read(ringbuf, 4, dest, 100, read, write);
		rbassert(got4 == 1);
		rbassert(win::RingBufferBase::size<T>(4, read, write) == 0);
		rbassert(read == 0);
		rbassert(write == 0);
		rbassert(dest[0] == 4 && dest[1] == canary && dest[2] == canary && dest[4] == canary);
		memcheck();
	}
}

int main()
{
	run<std::int16_t>();
	run<float>();
	run<char>();
	run<unsigned>();

	fprintf(stderr, "All %d asserts passed.\n", testcount);
}
