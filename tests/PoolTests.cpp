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

#define poolassert(exp) if (!(exp)) win::bug("pool assert failed (" + std::to_string(__LINE__) + "): " #exp)

static std::aligned_storage_t<sizeof(win::impl::PoolNode<TestObject>), alignof(win::impl::PoolNode<TestObject>)> *get_spot(TestObject &to)
{
	win::impl::PoolNode<TestObject> *node;
	node = static_cast<decltype(node)>(&to);

	return static_cast<std::aligned_storage_t<sizeof(win::impl::PoolNode<TestObject>), alignof(win::impl::PoolNode<TestObject>)>*>(node->spot);
}

template <int initial_capacity> void run(win::Pool<TestObject, initial_capacity> &pool)
{
	int num = 0;
	auto &red = pool.add(num, "red");
	auto &orange = pool.add(num, "orange");
	auto &yellow = pool.add(num, "yellow");
	auto &green = pool.add(num, "green");
	auto &blue = pool.add(num, "blue");
	auto &indigo = pool.add(num, "indigo");
	auto &violet = pool.add(num, "violet");

	poolassert(pool.count == 7);
	poolassert(pool.freelist.size() == 0);

	{
		const win::Pool<TestObject, initial_capacity> &constpool = pool;
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

	poolassert(pool.count == 6);
	poolassert(pool.freelist.size() == 1);
	poolassert(pool.freelist.at(0) == get_spot(red));

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

	poolassert(pool.count == 4);
	poolassert(pool.freelist.size() == 3);
	poolassert(pool.freelist.at(0) == get_spot(red));
	poolassert(pool.freelist.at(1) == get_spot(green));
	poolassert(pool.freelist.at(2) == get_spot(blue));

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

	poolassert(pool.count == 3);
	poolassert(pool.freelist.size() == 4);
	poolassert(pool.freelist.at(0) == get_spot(red));
	poolassert(pool.freelist.at(1) == get_spot(green));
	poolassert(pool.freelist.at(2) == get_spot(blue));
	poolassert(pool.freelist.at(3) == get_spot(violet));

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

		poolassert(pool.count == 4);
		poolassert(pool.freelist.size() == 3);
		poolassert(pool.freelist.at(0) == get_spot(red));
		poolassert(pool.freelist.at(1) == get_spot(green));
		poolassert(pool.freelist.at(2) == get_spot(blue));

		pool.remove(rainbow);

		auto it2 = pool.begin();

		poolassert(it2->name == "orange");
		it2++;
		poolassert(it2->name == "yellow");
		it2++;
		poolassert(it2->name == "indigo");
		it2++;
		poolassert(it2 == pool.end());

		poolassert(pool.count == 3);
		poolassert(pool.freelist.size() == 4);
		poolassert(pool.freelist.at(0) == get_spot(red));
		poolassert(pool.freelist.at(1) == get_spot(green));
		poolassert(pool.freelist.at(2) == get_spot(blue));
		poolassert(pool.freelist.at(3) == get_spot(rainbow));
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

	poolassert(pool.count == 1);
	poolassert(pool.freelist.size() == 6);
	poolassert(pool.freelist.at(0) == get_spot(red));
	poolassert(pool.freelist.at(1) == get_spot(green));
	poolassert(pool.freelist.at(2) == get_spot(blue));
	poolassert(pool.freelist.at(3) == get_spot(violet));
	poolassert(pool.freelist.at(4) == get_spot(yellow));
	poolassert(pool.freelist.at(5) == get_spot(orange));

	{
		auto it = pool.begin();

		poolassert(it->name == "indigo");
		++it;
		poolassert(it == pool.end());
	}

	pool.remove(indigo);

	poolassert(pool.count == 0);
	poolassert(pool.freelist.size() == 0);

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

int main()
{
	{
		win::Pool<TestObject> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 1> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 2> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 3> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 4> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 5> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 6> pool;
		run(pool);
	}

	{
		win::Pool<TestObject, 7> pool;
		run(pool);
	}

	fprintf(stderr, "All tests ran successfully\n");

	return 0;
}
