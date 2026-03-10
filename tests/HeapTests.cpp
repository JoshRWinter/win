#include <string>

#include <win/Win.hpp>
#define private public
#include <win/Heap.hpp>
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

template<int cap, bool inl> unsigned char *get_spot(win::Heap<TestObject, cap, inl> &heap, int spot)
{
	int partition_no = spot / cap;
	int index = spot % cap;

	win::impl::HeapPartition<TestObject, cap> *partition = heap.get_first_partition();
	for (int i = 0; i < partition_no; ++i)
		partition = partition->next.get();

	return partition->storage[index].item;
}

#define heapassert(exp)                                                                                                                                        \
	if (!(exp))                                                                                                                                                \
	win::bug("heap assert failed (" + std::to_string(__LINE__) + "): " #exp)

template<int partition_capacity, bool first_partition_inline> void run(win::Heap<TestObject, partition_capacity, first_partition_inline> &heap)
{
	int num = 0;

	auto &red = heap.add(num, "red");
	auto &orange = heap.add(num, "orange");
	auto &yellow = heap.add(num, "yellow");
	auto &green = heap.add(num, "green");
	auto &blue = heap.add(num, "blue");
	auto &indigo = heap.add(num, "indigo");
	auto &violet = heap.add(num, "violet");

	heapassert(heap.count == 7);
	heapassert(heap.freelist.size() == 0);

	heap.remove(red);

	heapassert(heap.count == 6);

	heapassert(heap.freelist.size() == 1);
	heapassert(heap.freelist.at(0) == get_spot(heap, 0));

	heap.remove(green);
	heap.remove(blue);

	heapassert(heap.count == 4);

	heapassert(heap.freelist.size() == 3);
	heapassert(heap.freelist.at(0) == get_spot(heap, 0));
	heapassert(heap.freelist.at(1) == get_spot(heap, 3));
	heapassert(heap.freelist.at(2) == get_spot(heap, 4));

	heap.remove(violet);

	heapassert(heap.count == 3);

	heapassert(heap.freelist.size() == 4);
	heapassert(heap.freelist.at(0) == get_spot(heap, 0));
	heapassert(heap.freelist.at(1) == get_spot(heap, 3));
	heapassert(heap.freelist.at(2) == get_spot(heap, 4));
	heapassert(heap.freelist.at(3) == get_spot(heap, 6));

	{
		auto &rainbow = heap.add(num, "rainbow");

		heapassert(heap.count == 4);

		heapassert(heap.freelist.size() == 3);
		heapassert(heap.freelist.at(0) == get_spot(heap, 0));
		heapassert(heap.freelist.at(1) == get_spot(heap, 3));
		heapassert(heap.freelist.at(2) == get_spot(heap, 4));

		heap.remove(rainbow);

		heapassert(heap.count == 3);

		heapassert(heap.freelist.size() == 4);
		heapassert(heap.freelist.at(0) == get_spot(heap, 0));
		heapassert(heap.freelist.at(1) == get_spot(heap, 3));
		heapassert(heap.freelist.at(2) == get_spot(heap, 4));
		heapassert(heap.freelist.at(3) == get_spot(heap, 6));
	}

	heap.remove(yellow);
	heap.remove(orange);

	heapassert(heap.count == 1);

	heapassert(heap.freelist.size() == 6);
	heapassert(heap.freelist.at(0) == get_spot(heap, 0));
	heapassert(heap.freelist.at(1) == get_spot(heap, 3));
	heapassert(heap.freelist.at(2) == get_spot(heap, 4));
	heapassert(heap.freelist.at(3) == get_spot(heap, 6));
	heapassert(heap.freelist.at(4) == get_spot(heap, 2));
	heapassert(heap.freelist.at(5) == get_spot(heap, 1));

	heap.remove(indigo);

	heapassert(heap.count == 0);
	heapassert(heap.freelist.size() == 0);

	heapassert(num == 0);
	heapassert(heap.size() == 0);

	auto one = heap.add(num, "one");
	auto two = heap.add(num, "two");
	auto three = heap.add(num, "three");

	heapassert(num == 3);
	heapassert(heap.size() == 3);

	heap.remove(three);
	heap.remove(two);
	heap.remove(one);

	heapassert(num == 0);
	heapassert(heap.size() == 0);
}

template<bool first_partition_inline> void run()
{
	{
		win::Heap<TestObject, 1, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 2, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 3, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 4, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 5, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 6, first_partition_inline> heap;
		run(heap);
	}

	{
		win::Heap<TestObject, 7, first_partition_inline> heap;
		run(heap);
	}
}

int main()
{
	run<false>();
	run<true>();

	fprintf(stderr, "All tests ran successfully\n");

	return 0;
}
