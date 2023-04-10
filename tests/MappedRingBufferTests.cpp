#define private public
#include <win/MappedRingBuffer.hpp>

int successfull = 0;
#define assert(exp) do { if (!(exp)) { win::bug("Assert failed on line " + std::to_string(__LINE__)); } else { ++successfull; } } while (false)

template <typename T> void test_mapped_buffer()
{
	T mapped[5];
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 0);
		assert(range.original_head == 0);
		assert(range.original_length == 5);
		assert(range.current_head == 0);
		assert(range.current_remaining == 5);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 0);
		assert(range.original_length == 3);
		assert(range.current_head == 0);
		assert(range.current_remaining == 3);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 3);
		assert(range.original_length == 4);
		assert(range.current_head == 3);
		assert(range.current_remaining == 4);
	}

	{
		auto range = mbuf.reserve(2);

		assert(mbuf.buffer_head == 4);
		assert(range.original_head == 2);
		assert(range.original_length == 2);
		assert(range.current_head == 2);
		assert(range.current_remaining == 2);
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 4);
		assert(range.original_head == 4);
		assert(range.original_length == 5);
		assert(range.current_head == 4);
		assert(range.current_remaining == 5);
	}
}

template <typename T> void test_mapped_buffer_range_write()
{
	T mapped[5] { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 0);
		assert(range.original_length == 3);
		assert(range.current_head == 0);
		assert(range.current_remaining == 3);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 3);
		assert(range.original_length == 4);
		assert(range.current_head == 3);
		assert(range.current_remaining == 4);

		{
			const T src[2] { 2, 66 };
			const int wrote = range.write(src, 2);

			assert(wrote == 2);
			assert(range.current_head == 0);
			assert(range.current_remaining == 2);
			assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 48 };
			const int wrote = range.write(src, 1);

			assert(wrote == 1);
			assert(range.current_head == 1);
			assert(range.current_remaining == 1);
			assert(mapped[0] == 48 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 65 };
			const int wrote = range.write(src, 1);

			assert(wrote == 1);
			assert(range.current_head == 2);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 48 && mapped[1] == 65 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 2);
		assert(range.original_length == 5);
		assert(range.current_head == 2);
		assert(range.current_remaining == 5);

		{
			T src[0];
			const int wrote = range.write(src, 0);

			assert(wrote == 0);
			assert(range.current_head == 2);
			assert(range.current_remaining == 5);
			assert(mapped[0] == 48 && mapped[1] == 65 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[4] = { 4, 96, 54, 41 };
			const int wrote = range.write(src, 4);

			assert(wrote == 4);
			assert(range.current_head == 1);
			assert(range.current_remaining == 1);
			assert(mapped[0] == 41 && mapped[1] == 65 && mapped[2] == 4 && mapped[3] == 96 && mapped[4] == 54);
		}

		{
			const T src[1] = { 101 };
			const int wrote = range.write(src, 1);

			assert(wrote == 1);
			assert(range.current_head == 2);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 41 && mapped[1] == 101 && mapped[2] == 4 && mapped[3] == 96 && mapped[4] == 54);
		}
	}

	{
		auto range = mbuf.reserve(0);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 2);
		assert(range.original_length == 0);
		assert(range.current_head == 2);
		assert(range.current_remaining == 0);
	}
}

template <typename T> void test_iterator()
{
	T mapped[5] = { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);
	assert(mbuf.buffer_head == 0);

	{
		auto range = mbuf.reserve(2);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 0);
		assert(range.original_length == 2);
		assert(range.current_head == 0);
		assert(range.current_remaining == 2);

		{
			auto it = range.begin();

			assert(range.current_head == 0);
			assert(range.current_remaining == 2);

			*it = 67;
			++it;

			assert(it != range.end());
			assert(range.current_head == 1);
			assert(range.current_remaining == 1);
			assert(mapped[0] == 67 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);

			*it = 12;
			++it;

			assert(it == range.end());
			assert(range.current_head == 2);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 67 && mapped[1] == 12 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve(1);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 2);
		assert(range.original_length == 1);
		assert(range.current_head == 2);
		assert(range.current_remaining == 1);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.original_head == 3);
		assert(range.original_length == 4);
		assert(range.current_head == 3);
		assert(range.current_remaining == 4);

		int index = 0;
		for (T &item : range)
		{
			assert(range.current_head == (3 + index) % 5);
			assert(range.current_remaining == 4 - index);
			item = index * 2;
			++index;
		}

		assert(mapped[0] == 4 && mapped[1] == 6 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 2);
	}
}

template <typename T> void test_subscript_operator()
{
	T mapped[5] = { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);
	assert(mbuf.buffer_head == 0);

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 0);
		assert(range.original_head == 0);
		assert(range.original_length == 5);
		assert(range.current_head == 0);
		assert(range.current_remaining == 5);

		range[4] = 91;

		assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 91);

		range[1] = 2;

		assert(mapped[0] == 0 && mapped[1] == 2 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 91);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 0);
		assert(range.original_length == 3);
		assert(range.current_head == 0);
		assert(range.current_remaining == 3);

		range[0] = 5;
		range[1] = 93;
		range[2] = 44;

		assert(mapped[0] == 5 && mapped[1] == 93 && mapped[2] == 44 && mapped[3] == 0 && mapped[4] == 91);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 1);
		assert(range.original_head == 3);
		assert(range.original_length == 3);
		assert(range.current_head == 3);
		assert(range.current_remaining == 3);

		range[0] = 56;
		range[1] = 1;
		range[2] = 39;

		assert(mapped[0] == 39 && mapped[1] == 93 && mapped[2] == 44 && mapped[3] == 56 && mapped[4] == 1);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 0);
		assert(range.original_head == 1);
		assert(range.original_length == 4);
		assert(range.current_head == 1);
		assert(range.current_remaining == 4);

		range[0] = 7;
		range[1] = 81;
		range[2] = 72;
		range[3] = 60;

		assert(mapped[0] == 39 && mapped[1] == 7 && mapped[2] == 81 && mapped[3] == 72 && mapped[4] == 60);
	}

	{
		auto range = mbuf.reserve(0);

		assert(mbuf.buffer_head == 0);
		assert(range.original_head == 0);
		assert(range.original_length == 0);
		assert(range.current_head == 0);
		assert(range.current_remaining == 0);

		// range[0] = 3; // would abort
	}
}

template <typename T> void test_mixed()
{
	T mapped[5] = { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);
	assert(mbuf.buffer_head == 0);

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 0);
		assert(range.original_head == 0);
		assert(range.original_length == 5);
		assert(range.current_head == 0);
		assert(range.current_remaining == 5);

		{
			auto it = range.begin();

			assert(it != range.end());

			*it = 48;
			++it;

			assert(range.current_head == 1);
			assert(range.current_remaining == 4);
			assert(mapped[0] == 48 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);
		}

		{
			const T src[] = {88, 120, 47 };
			const int wrote = range.write(src, 3);

			assert(wrote == 3);
			assert(range.current_head == 4);
			assert(range.current_remaining == 1);
			assert(mapped[0] == 48 && mapped[1] == 88 && mapped[2] == 120 && mapped[3] == 47 && mapped[4] == 0);
		}

		{
			int iteration = 0;
			for (auto &item : range)
			{
				item = 3;
				++iteration;
			}

			assert(iteration == 1);
			assert(range.current_head == 0);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 48 && mapped[1] == 88 && mapped[2] == 120 && mapped[3] == 47 && mapped[4] == 3);
		}

		{
			auto it = range.begin();
			assert(it == range.end());
		}

		range[2] = 58;

		assert(mapped[0] == 48 && mapped[1] == 88 && mapped[2] == 58 && mapped[3] == 47 && mapped[4] == 3);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 0);
		assert(range.original_length == 3);
		assert(range.current_head == 0);
		assert(range.current_remaining == 3);
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 3);
		assert(range.original_length == 5);
		assert(range.current_head == 3);
		assert(range.current_remaining == 5);

		{
			const T src[] = { 65, 27, 88 };
			const int wrote = range.write(src, 3);

			assert(wrote == 3);
			assert(range.current_head == 1);
			assert(range.current_remaining == 2);
			assert(mapped[0] == 88 && mapped[1] == 88 && mapped[2] == 58 && mapped[3] == 65 && mapped[4] == 27);
		}

		range[1] = 102;

		{
			int iteration = 0;
			for (auto &item : range)
			{
				item = (iteration + 1) * 4;

				assert(range.current_head == (1 + iteration) % 5);
				assert(range.current_remaining == 2 - iteration);

				++iteration;
			}

			assert(range.current_head == 3);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 88 && mapped[1] == 4 && mapped[2] == 8 && mapped[3] == 65 && mapped[4] == 102);
		}
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 3);
		assert(range.original_head == 3);
		assert(range.original_length == 5);
		assert(range.current_head == 3);
		assert(range.current_remaining == 5);

		{
			auto it = range.begin();

			assert(it != range.end());

			*it = 99;
			it++;

			assert(it != range.end());
			assert(range.current_head == 4);
			assert(range.current_remaining == 4);
			assert(mapped[0] == 88 && mapped[1] == 4 && mapped[2] == 8 && mapped[3] == 99 && mapped[4] == 102);

			*it = 41;
			++it;

			assert(it != range.end());
			assert(range.current_head == 0);
			assert(range.current_remaining == 3);
			assert(mapped[0] == 88 && mapped[1] == 4 && mapped[2] == 8 && mapped[3] == 99 && mapped[4] == 41);

			*it = 125;
			++it;

			assert(it != range.end());
			assert(range.current_head == 1);
			assert(range.current_remaining == 2);
			assert(mapped[0] == 125 && mapped[1] == 4 && mapped[2] == 8 && mapped[3] == 99 && mapped[4] == 41);
		}

		{
			const T src[] = { 64, 32 };

			range.write(src, 2);

			assert(range.current_head == 3);
			assert(range.current_remaining == 0);
			assert(mapped[0] == 125 && mapped[1] == 64 && mapped[2] == 32 && mapped[3] == 99 && mapped[4] == 41);
		}

		range[0] = 2;
		range[4] = 95;

		assert(mapped[0] == 125 && mapped[1] == 64 && mapped[2] == 95 && mapped[3] == 2 && mapped[4] == 41);
	}
}

int main()
{
	test_mapped_buffer<unsigned char>();
	test_mapped_buffer<int>();
	test_mapped_buffer<float>();
	test_mapped_buffer<double>();

	test_mapped_buffer_range_write<unsigned char>();
	test_mapped_buffer_range_write<int>();
	test_mapped_buffer_range_write<float>();
	test_mapped_buffer_range_write<double>();

	test_iterator<unsigned char>();
	test_iterator<int>();
	test_iterator<float>();
	test_iterator<double>();

	test_subscript_operator<unsigned char>();
	test_subscript_operator<int>();
	test_subscript_operator<float>();
	test_subscript_operator<double>();

	test_mixed<unsigned char>();
	test_mixed<int>();
	test_mixed<float>();
	test_mixed<double>();

	fprintf(stderr, "%d asserts passed\n", successfull);
}
