#include <string>

#include <win/Win.hpp>
#define private public
#include <win/Bag.hpp>
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

template<int cap, bool inl> unsigned char *get_spot(win::Bag<TestObject, cap, inl> &bag, int spot)
{
    int partition_no = spot / cap;
    int index = spot % cap;

    win::impl::BagPartition<TestObject, cap> *partition = bag.get_first_partition();
    for (int i = 0; i < partition_no; ++i)
        partition = partition->next.get();

    return partition->storage[index].item;
}

#define bagassert(exp)                                                                                                                                         \
    if (!(exp))                                                                                                                                                \
    win::bug("bag assert failed (" + std::to_string(__LINE__) + "): " #exp)

template<int partition_capacity, bool first_partition_inline> void run(win::Bag<TestObject, partition_capacity, first_partition_inline> &bag)
{
    int num = 0;

    auto &red = bag.add(num, "red");
    auto &orange = bag.add(num, "orange");
    auto &yellow = bag.add(num, "yellow");
    auto &green = bag.add(num, "green");
    auto &blue = bag.add(num, "blue");
    auto &indigo = bag.add(num, "indigo");
    auto &violet = bag.add(num, "violet");

    bagassert(bag.count == 7);
    bagassert(bag.freelist.size() == 0);

    bag.remove(red);

    bagassert(bag.count == 6);

    bagassert(bag.freelist.size() == 1);
    bagassert(bag.freelist.at(0) == get_spot(bag, 0));

    bag.remove(green);
    bag.remove(blue);

    bagassert(bag.count == 4);

    bagassert(bag.freelist.size() == 3);
    bagassert(bag.freelist.at(0) == get_spot(bag, 0));
    bagassert(bag.freelist.at(1) == get_spot(bag, 3));
    bagassert(bag.freelist.at(2) == get_spot(bag, 4));

    bag.remove(violet);

    bagassert(bag.count == 3);

    bagassert(bag.freelist.size() == 4);
    bagassert(bag.freelist.at(0) == get_spot(bag, 0));
    bagassert(bag.freelist.at(1) == get_spot(bag, 3));
    bagassert(bag.freelist.at(2) == get_spot(bag, 4));
    bagassert(bag.freelist.at(3) == get_spot(bag, 6));

    {
        auto &rainbow = bag.add(num, "rainbow");

        bagassert(bag.count == 4);

        bagassert(bag.freelist.size() == 3);
        bagassert(bag.freelist.at(0) == get_spot(bag, 0));
        bagassert(bag.freelist.at(1) == get_spot(bag, 3));
        bagassert(bag.freelist.at(2) == get_spot(bag, 4));

        bag.remove(rainbow);

        bagassert(bag.count == 3);

        bagassert(bag.freelist.size() == 4);
        bagassert(bag.freelist.at(0) == get_spot(bag, 0));
        bagassert(bag.freelist.at(1) == get_spot(bag, 3));
        bagassert(bag.freelist.at(2) == get_spot(bag, 4));
        bagassert(bag.freelist.at(3) == get_spot(bag, 6));
    }

    bag.remove(yellow);
    bag.remove(orange);

    bagassert(bag.count == 1);

    bagassert(bag.freelist.size() == 6);
    bagassert(bag.freelist.at(0) == get_spot(bag, 0));
    bagassert(bag.freelist.at(1) == get_spot(bag, 3));
    bagassert(bag.freelist.at(2) == get_spot(bag, 4));
    bagassert(bag.freelist.at(3) == get_spot(bag, 6));
    bagassert(bag.freelist.at(4) == get_spot(bag, 2));
    bagassert(bag.freelist.at(5) == get_spot(bag, 1));

    bag.remove(indigo);

    bagassert(bag.count == 0);
    bagassert(bag.freelist.size() == 0);

    bagassert(num == 0);
    bagassert(bag.size() == 0);

    auto one = bag.add(num, "one");
    auto two = bag.add(num, "two");
    auto three = bag.add(num, "three");

    bagassert(num == 3);
    bagassert(bag.size() == 3);

    bag.remove(three);
    bag.remove(two);
    bag.remove(one);

    bagassert(num == 0);
    bagassert(bag.size() == 0);
}

template<bool first_partition_inline> void run()
{
    {
        win::Bag<TestObject, 1, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 2, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 3, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 4, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 5, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 6, first_partition_inline> bag;
        run(bag);
    }

    {
        win::Bag<TestObject, 7, first_partition_inline> bag;
        run(bag);
    }
}

int main()
{
    run<false>();
    run<true>();

    fprintf(stderr, "All tests ran successfully\n");

    return 0;
}
