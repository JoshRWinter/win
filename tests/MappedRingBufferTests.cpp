#include <vector>

#define private public
#include <win/MappedRingBuffer.hpp>

int successfull = 0;
#define assert(exp) do { if (!(exp)) { win::bug("Assert failed on line " + std::to_string(__LINE__)); } else { ++successfull; } } while (false)

template <typename T> std::vector<win::MappedRingBufferRangeSlice> vec(const win::MappedRingBufferRange<T> &range)
{
	std::vector<win::MappedRingBufferRangeSlice> v;
	for (const auto &slice : range.slices())
		v.emplace_back(slice);
	return v;
}

template <typename T> void test_mapped_buffer()
{
	T mapped[5];
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 0);
		assert(range.range_length == 5);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 5);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 3);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 3);
		assert(range.range_length == 4);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 3);
		assert(vec(range).at(0).length == 2);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 2);
	}

	{
		auto range = mbuf.reserve(2);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 2);
		assert(range.range_length == 2);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 2);
		assert(vec(range).at(0).length == 2);
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 4);
		assert(range.range_length == 5);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 4);
		assert(vec(range).at(0).length == 1);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 4);
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
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 3);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 3);
		assert(range.range_length == 4);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 3);
		assert(vec(range).at(0).length == 2);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 2);

		{
			const T src[2] { 2, 66 };
			const int wrote = range.write(src, 2);

			assert(wrote == 2);
			assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 48 };
			const int wrote = range.write(2, src, 1);

			assert(wrote == 1);
			assert(mapped[0] == 48 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 65 };
			const int wrote = range.write(3, src, 1);

			assert(wrote == 1);
			assert(mapped[0] == 48 && mapped[1] == 65 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 2);
		assert(range.range_length == 5);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 2);
		assert(vec(range).at(0).length == 3);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 2);

		{
			T src[0];
			const int wrote = range.write(src, 0);

			assert(wrote == 0);
			assert(mapped[0] == 48 && mapped[1] == 65 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[4] = { 4, 96, 54, 41 };
			const int wrote = range.write(src, 4);

			assert(wrote == 4);
			assert(mapped[0] == 41 && mapped[1] == 65 && mapped[2] == 4 && mapped[3] == 96 && mapped[4] == 54);
		}

		{
			const T src[1] = { 101 };
			const int wrote = range.write(4, src, 1);

			assert(wrote == 1);
			assert(mapped[0] == 41 && mapped[1] == 101 && mapped[2] == 4 && mapped[3] == 96 && mapped[4] == 54);
		}
	}

	{
		auto range = mbuf.reserve(0);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 2);
		assert(range.range_length == 0);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 2);
		assert(vec(range).at(0).length == 0);
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
		assert(range.range_head == 0);
		assert(range.range_length == 2);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 2);

		{
			auto it = range.begin();

			assert(it.position == 0);

			*it = 67;
			++it;

			assert(it.position == 1);
			assert(it != range.end());
			assert(mapped[0] == 67 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);

			*it = 12;
			++it;

			assert(it.position == 2);
			assert(it == range.end());
			assert(mapped[0] == 67 && mapped[1] == 12 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve(1);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 2);
		assert(range.range_length == 1);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 2);
		assert(vec(range).at(0).length == 1);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 3);
		assert(range.range_length == 4);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 3);
		assert(vec(range).at(0).length == 2);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 2);

		int index = 0;
		for (T &item : range)
		{
			item = index * 2;
			++index;
		}

		assert(mapped[0] == 4 && mapped[1] == 6 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 2);
	}

	{
		auto range = mbuf.reserve(5);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 2);
		assert(range.range_length == 5);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 2);
		assert(vec(range).at(0).length == 3);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 2);

		{
			auto it = range.begin() + 4;

			assert(it.position == 4);

			*it = 58;
			it = it++;

			assert(it.position == 4);
			assert(it != range.end());
			assert(mapped[0] == 4 && mapped[1] == 58 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 2);

			*it = 69;

			assert(mapped[0] == 4 && mapped[1] == 69 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 2);

			it = it - 3;
			*it = 70;

			assert(it.position == 1);
			assert(mapped[0] == 4 && mapped[1] == 69 && mapped[2] == 0 && mapped[3] == 70 && mapped[4] == 2);

			--it;
			*it = 9;

			assert(it.position == 0);
			assert(mapped[0] == 4 && mapped[1] == 69 && mapped[2] == 9 && mapped[3] == 70 && mapped[4] == 2);

			it = it + 3;

			assert(it.position == 3);

			*it = 20;

			assert(mapped[0] == 20 && mapped[1] == 69 && mapped[2] == 9 && mapped[3] == 70 && mapped[4] == 2);

			it = --it;
			*it = 90;

			assert(it.position == 2);
			assert(mapped[0] == 20 && mapped[1] == 69 && mapped[2] == 9 && mapped[3] == 70 && mapped[4] == 90);

			it = it--;
			*it = 40;

			assert(it.position == 2);
			assert(mapped[0] == 20 && mapped[1] == 69 && mapped[2] == 9 && mapped[3] == 70 && mapped[4] == 40);

			it = ++it;
			*it = 53;

			assert(it.position == 3);
			assert(mapped[0] == 53 && mapped[1] == 69 && mapped[2] == 9 && mapped[3] == 70 && mapped[4] == 40);
		}
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
		assert(range.range_head == 0);
		assert(range.range_length == 5);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 5);

		range[4] = 91;

		assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 91);

		range[1] = 2;

		assert(mapped[0] == 0 && mapped[1] == 2 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 91);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 3);

		range[0] = 5;
		range[1] = 93;
		range[2] = 44;

		assert(mapped[0] == 5 && mapped[1] == 93 && mapped[2] == 44 && mapped[3] == 0 && mapped[4] == 91);
	}

	{
		auto range = mbuf.reserve(3);

		assert(mbuf.buffer_head == 1);
		assert(range.range_head == 3);
		assert(range.range_length == 3);

		assert(vec(range).size() == 2);
		assert(vec(range).at(0).start == 3);
		assert(vec(range).at(0).length == 2);
		assert(vec(range).at(1).start == 0);
		assert(vec(range).at(1).length == 1);

		range[0] = 56;
		range[1] = 1;
		range[2] = 39;

		assert(mapped[0] == 39 && mapped[1] == 93 && mapped[2] == 44 && mapped[3] == 56 && mapped[4] == 1);
	}

	{
		auto range = mbuf.reserve(4);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 1);
		assert(range.range_length == 4);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 1);
		assert(vec(range).at(0).length == 4);

		range[0] = 7;
		range[1] = 81;
		range[2] = 72;
		range[3] = 60;

		assert(mapped[0] == 39 && mapped[1] == 7 && mapped[2] == 81 && mapped[3] == 72 && mapped[4] == 60);
	}

	{
		auto range = mbuf.reserve(0);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 0);
		assert(range.range_length == 0);

		assert(vec(range).size() == 1);
		assert(vec(range).at(0).start == 0);
		assert(vec(range).at(0).length == 0);

		// range[0] = 3; // would abort
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

	fprintf(stderr, "%d asserts passed\n", successfull);
}
