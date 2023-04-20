#include <vector>

#define private public
#define protected public
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

template <typename T> void test_mapped_buffer_reserve()
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
			range.write(src, 2);

			assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 48 };
			range.write(2, src, 1);

			assert(mapped[0] == 48 && mapped[1] == 0 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[1] { 65 };
			range.write(3, src, 1);

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
			range.write(src, 0);

			assert(mapped[0] == 48 && mapped[1] == 65 && mapped[2] == 0 && mapped[3] == 2 && mapped[4] == 66);
		}

		{
			const T src[4] = { 4, 96, 54, 41 };
			range.write(src, 4);

			assert(mapped[0] == 41 && mapped[1] == 65 && mapped[2] == 4 && mapped[3] == 96 && mapped[4] == 54);
		}

		{
			const T src[1] = { 101 };
			range.write(4, src, 1);

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

template <typename T> void test_mapped_buffer_range_iterator()
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

template <typename T> void test_mapped_buffer_range_subscript_operator()
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

template <typename T> void test_mapped_buffer_contiguous_reserve()
{
	T mapped[5];
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);

	{
		auto range = mbuf.reserve_contiguous(5);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 0);
		assert(range.range_length == 5);
	}

	{
		auto range = mbuf.reserve_contiguous(1);

		assert(mbuf.buffer_head == 1);
		assert(range.range_head == 0);
		assert(range.range_length == 1);
	}

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 1);
		assert(range.range_length == 3);
	}

	{
		auto range = mbuf.reserve_contiguous(1);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 4);
		assert(range.range_length == 1);
	}

	{
		auto range = mbuf.reserve_contiguous(4);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 0);
		assert(range.range_length == 4);
	}

	{
		auto range = mbuf.reserve_contiguous(2);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 0);
		assert(range.range_length == 2);
	}

	{
		auto range = mbuf.reserve_contiguous(1);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 2);
		assert(range.range_length == 1);
	}

	{
		auto range = mbuf.reserve_contiguous(4);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 0);
		assert(range.range_length == 4);
	}
}

template <typename T> void test_mapped_buffer_range_contiguous_write()
{
	T mapped[5] { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		{
			const T src[] = { 39, 45 };
			range.write(src, 2);

			assert(mapped[0] == 39 && mapped[1] == 45 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);
		}

		{
			const T src[] = { 99, 77 };
			range.write(1, src, 2);

			assert(mapped[0] == 39 && mapped[1] == 99 && mapped[2] == 77 && mapped[3] == 0 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		{
			const T src[] = { 69, 5 };
			range.write(src, 2);

			assert(mapped[0] == 69 && mapped[1] == 5 && mapped[2] == 77 && mapped[3] == 0 && mapped[4] == 0);
		}

		{
			const T src[] = { 40 };
			range.write(2, src, 1);

			assert(mapped[0] == 69 && mapped[1] == 5 && mapped[2] == 40 && mapped[3] == 0 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve_contiguous(1);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 3);
		assert(range.range_length == 1);

		{
			const T src[] = { 3 };
			range.write(src, 1);

			assert(mapped[0] == 69 && mapped[1] == 5 && mapped[2] == 40 && mapped[3] == 3 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve_contiguous(2);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 0);
		assert(range.range_length == 2);

		{
			const T src[] = { 55, 39  };
			range.write(src, 2);

			assert(mapped[0] == 55 && mapped[1] == 39 && mapped[2] == 40 && mapped[3] == 3 && mapped[4] == 0);
		}
	}
}

template <typename T> void test_mapped_buffer_contiguous_range_iterator()
{
	T mapped[5] = { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);
	assert(mbuf.buffer_head == 0);

	{
		auto range = mbuf.reserve_contiguous(2);

		assert(mbuf.buffer_head == 2);
		assert(range.range_head == 0);
		assert(range.range_length == 2);

		int index = 0;
		for (auto &item : range)
		{
			item = index * 3;
			++index;
		}

		assert(mapped[0] == 0 && mapped[1] == 3 && mapped[2] == 0 && mapped[3] == 0 && mapped[4] == 0);
	}

	{
		auto range = mbuf.reserve_contiguous(4);

		assert(mbuf.buffer_head == 4);
		assert(range.range_head == 0);
		assert(range.range_length == 4);

		int index = 4;
		for (auto &item : range)
		{
			item = index * 4;
			++index;
		}

		assert(mapped[0] == 16 && mapped[1] == 20 && mapped[2] == 24 && mapped[3] == 28 && mapped[4] == 0);
	}

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		{
			auto it = range.begin();

			assert(it.position == 0);

			*it = 5;
			++it;

			assert(it.position == 1);
			assert(mapped[0] == 5 && mapped[1] == 20 && mapped[2] == 24 && mapped[3] == 28 && mapped[4] == 0);

			*it = 9;
			++it;

			assert(it.position == 2);
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 24 && mapped[3] == 28 && mapped[4] == 0);

			*it = 82;
			++it;

			assert(it == range.end());
			assert(it.position == 3);
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 82 && mapped[3] == 28 && mapped[4] == 0);
		}
	}

	{
		auto range = mbuf.reserve_contiguous(2);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 3);
		assert(range.range_length == 2);

		{
			auto it = range.begin();

			assert(it.position == 0);

			*it = 51;
			++it;

			assert(it.position == 1);
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 82 && mapped[3] == 51 && mapped[4] == 0);

			*it = 106;
			++it;

			assert(it.position == 2);
			assert(it == range.end());
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 82 && mapped[3] == 51 && mapped[4] == 106);
		}
	}

	{
		auto range = mbuf.reserve_contiguous(5);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 0);
		assert(range.range_length == 5);

		{
			auto it = range.begin() + 2;

			assert(it.position == 2);

			*it = 98;
			it = it--;

			assert(it.position == 2);
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 98 && mapped[3] == 51 && mapped[4] == 106);

			*it = 44;
			it = (--it) - 1;

			assert(it.position == 0);
			assert(mapped[0] == 5 && mapped[1] == 9 && mapped[2] == 44 && mapped[3] == 51 && mapped[4] == 106);

			*it = 1;
			it = it + 4;

			assert(it.position == 4);
			assert(mapped[0] == 1 && mapped[1] == 9 && mapped[2] == 44 && mapped[3] == 51 && mapped[4] == 106);

			*it = 12;
			it++;

			assert(it.position == 5);
			assert(it == range.end());
			assert(mapped[0] == 1 && mapped[1] == 9 && mapped[2] == 44 && mapped[3] == 51 && mapped[4] == 12);
		}
	}
}

template <typename T> void test_mapped_buffer_contiguous_range_subscript_operator()
{
	T mapped[5] = { 0, 0, 0, 0, 0 };
	win::MappedRingBuffer<T> mbuf(mapped, 5);

	assert(mbuf.buffer == mapped);
	assert(mbuf.buffer_length == 5);
	assert(mbuf.buffer_head == 0);

	{
		auto range = mbuf.reserve_contiguous(5);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 0);
		assert(range.range_length == 5);

		range[2] = 63;

		assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 63 && mapped[3] == 0 && mapped[4] == 0);

		range[4] = 8;

		assert(mapped[0] == 0 && mapped[1] == 0 && mapped[2] == 63 && mapped[3] == 0 && mapped[4] == 8);
	}

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		range[0] = 65;

		assert(mapped[0] == 65 && mapped[1] == 0 && mapped[2] == 63 && mapped[3] == 0 && mapped[4] == 8);
	}

	{
		auto range = mbuf.reserve_contiguous(3);

		assert(mbuf.buffer_head == 3);
		assert(range.range_head == 0);
		assert(range.range_length == 3);

		range[1] = 89;

		assert(mapped[0] == 65 && mapped[1] == 89 && mapped[2] == 63 && mapped[3] == 0 && mapped[4] == 8);
	}

	{
		auto range = mbuf.reserve_contiguous(2);

		assert(mbuf.buffer_head == 0);
		assert(range.range_head == 3);
		assert(range.range_length == 2);

		range[0] = 13;
		range[1] = 19;

		assert(mapped[0] == 65 && mapped[1] == 89 && mapped[2] == 63 && mapped[3] == 13 && mapped[4] == 19);
	}
}

int main()
{
	// wrap tests

	test_mapped_buffer_reserve<unsigned char>();
	test_mapped_buffer_reserve<int>();
	test_mapped_buffer_reserve<float>();
	test_mapped_buffer_reserve<double>();

	test_mapped_buffer_range_write<unsigned char>();
	test_mapped_buffer_range_write<int>();
	test_mapped_buffer_range_write<float>();
	test_mapped_buffer_range_write<double>();

	test_mapped_buffer_range_iterator<unsigned char>();
	test_mapped_buffer_range_iterator<int>();
	test_mapped_buffer_range_iterator<float>();
	test_mapped_buffer_range_iterator<double>();

	test_mapped_buffer_range_subscript_operator<unsigned char>();
	test_mapped_buffer_range_subscript_operator<int>();
	test_mapped_buffer_range_subscript_operator<float>();
	test_mapped_buffer_range_subscript_operator<double>();

	// contiguous tests

	test_mapped_buffer_contiguous_reserve<unsigned char>();
	test_mapped_buffer_contiguous_reserve<int>();
	test_mapped_buffer_contiguous_reserve<float>();
	test_mapped_buffer_contiguous_reserve<double>();

	test_mapped_buffer_range_contiguous_write<unsigned char>();
	test_mapped_buffer_range_contiguous_write<int>();
	test_mapped_buffer_range_contiguous_write<float>();
	test_mapped_buffer_range_contiguous_write<double>();

	test_mapped_buffer_contiguous_range_iterator<unsigned char>();
	test_mapped_buffer_contiguous_range_iterator<int>();
	test_mapped_buffer_contiguous_range_iterator<float>();
	test_mapped_buffer_contiguous_range_iterator<double>();

	test_mapped_buffer_contiguous_range_subscript_operator<unsigned char>();
	test_mapped_buffer_contiguous_range_subscript_operator<int>();
	test_mapped_buffer_contiguous_range_subscript_operator<float>();
	test_mapped_buffer_contiguous_range_subscript_operator<double>();

	fprintf(stderr, "%d asserts passed\n", successfull);
}
