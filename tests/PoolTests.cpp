#include <string>

#include <win/Win.hpp>
#define private public
#include <win/Pool.hpp>
#undef private

struct TestObject
{
	TestObject(int &number, const std::string &s)
		: number(number)
		, name(s)
	{
		number++;
	}

	~TestObject() { --number; }

	int &number;
	std::string name;
};

#define poolassert(exp)                                                                                                                                        \
	if (!(exp))                                                                                                                                                \
	win::bug("pool assert failed (" + std::to_string(__LINE__) + "): " #exp)

template<int partition_capacity, bool first_partition_inline, bool use_shared_heap> void run(win::Pool<TestObject,
																									   partition_capacity,
																									   first_partition_inline,
																									   use_shared_heap> &pool)
{
	int num = 0;

	auto &red = pool.add(num, "red");
	auto &orange = pool.add(num, "orange");
	auto &yellow = pool.add(num, "yellow");
	auto &green = pool.add(num, "green");
	auto &blue = pool.add(num, "blue");
	auto &indigo = pool.add(num, "indigo");
	auto &violet = pool.add(num, "violet");

	poolassert(pool.size() == 7);

	{
		const win::Pool<TestObject, partition_capacity, first_partition_inline, use_shared_heap> &constpool = pool;
		auto it = constpool.begin();

		poolassert(it->name == "red");
		++it;
		poolassert(it->name == "orange");
		++it;
		poolassert(it->name == "yellow");
		++it;
		poolassert(it->name == "green");
		++it;
		poolassert(it->name == "blue");
		++it;
		poolassert(it->name == "indigo");
		++it;
		poolassert(it->name == "violet");
		++it;
		poolassert(it == constpool.end());
	}

	pool.remove(red);

	poolassert(pool.size() == 6);

	{
		auto it = pool.begin();

		poolassert(it->name == "orange");
		++it;
		poolassert(it->name == "yellow");
		++it;
		poolassert(it->name == "green");
		++it;
		poolassert(it->name == "blue");
		++it;
		poolassert(it->name == "indigo");
		++it;
		poolassert(it->name == "violet");
		++it;
		poolassert(it == pool.end());
	}

	pool.remove(green);
	pool.remove(blue);

	poolassert(pool.size() == 4);

	{
		auto it = pool.begin();

		poolassert(it->name == "orange");
		++it;
		poolassert(it->name == "yellow");
		++it;
		poolassert(it->name == "indigo");
		++it;
		poolassert(it->name == "violet");
		++it;
		poolassert(it == pool.end());
	}

	pool.remove(violet);

	poolassert(pool.size() == 3);

	{
		auto &rainbow = pool.add(num, "rainbow");

		auto it = pool.begin();

		poolassert(it->name == "orange");
		it++;
		poolassert(it->name == "yellow");
		it++;
		poolassert(it->name == "indigo");
		it++;
		poolassert(it->name == "rainbow");
		it++;
		poolassert(it == pool.end());

		poolassert(pool.size() == 4);

		pool.remove(rainbow);

		auto it2 = pool.begin();

		poolassert(it2->name == "orange");
		it2++;
		poolassert(it2->name == "yellow");
		it2++;
		poolassert(it2->name == "indigo");
		it2++;
		poolassert(it2 == pool.end());

		poolassert(pool.size() == 3);
	}

	{
		auto it = pool.begin();

		poolassert(it->name == "orange");
		it++;
		poolassert(it->name == "yellow");
		it++;
		poolassert(it->name == "indigo");
		it++;
		poolassert(it == pool.end());
	}

	pool.remove(yellow);
	pool.remove(orange);

	poolassert(pool.size() == 1);

	{
		auto it = pool.begin();

		poolassert(it->name == "indigo");
		++it;
		poolassert(it == pool.end());
	}

	pool.remove(indigo);

	poolassert(pool.size() == 0);

	{
		auto it = pool.begin();
		poolassert(it == pool.end());
	}

	poolassert(num == 0);
	poolassert(pool.size() == 0);

	pool.add(num, "one");
	pool.add(num, "two");
	pool.add(num, "three");

	poolassert(num == 3);
	poolassert(pool.size() == 3);

	pool.clear();

	poolassert(num == 0);
	poolassert(pool.size() == 0);
}

template<bool first_partition_inline> void run()
{
	{
		win::Pool<TestObject, 1, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 1, first_partition_inline> heap;
		win::Pool<TestObject, 1, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 2, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 2, first_partition_inline> heap;
		win::Pool<TestObject, 2, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 3, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 3, first_partition_inline> heap;
		win::Pool<TestObject, 3, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 4, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 4, first_partition_inline> heap;
		win::Pool<TestObject, 4, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 5, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 5, first_partition_inline> heap;
		win::Pool<TestObject, 5, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 6, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 6, first_partition_inline> heap;
		win::Pool<TestObject, 6, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}

	{
		win::Pool<TestObject, 7, first_partition_inline> pool;
		run(pool);
		run(pool);
	}

	{
		win::Heap<win::PoolNode<TestObject>, 7, first_partition_inline> heap;
		win::Pool<TestObject, 7, first_partition_inline, true> pool(heap);
		run(pool);
		run(pool);
	}
}

int main()
{
	run<false>();
	run<true>();

	fprintf(stderr, "All tests ran successfully\n");

	return 0;
}
