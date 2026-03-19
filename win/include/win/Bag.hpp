#pragma once

#include <limits>
#include <memory>
#include <vector>

#include <win/Win.hpp>

namespace win
{

namespace impl
{

template<typename T> struct BagItem
{
    alignas(alignof(T)) unsigned char item[sizeof(T)];
};

template<typename T, int capacity> struct BagPartition
{
    WIN_NO_COPY_MOVE(BagPartition);

    BagPartition() = default;

    std::unique_ptr<BagPartition<T, capacity>> next;
    BagItem<T> storage[capacity];
};

}

template<typename T, int partition_capacity, bool first_partition_inline> class Bag
{
    WIN_NO_COPY(Bag);

public:
    constexpr static int bag_partition_capacity = partition_capacity;
    constexpr static int bag_first_partition_inline = first_partition_inline;

    Bag()
    {
        static_assert(partition_capacity > 0, "Capacity must be greater than zero.");

        if constexpr (!first_partition_inline)
            first_partition.reset(new impl::BagPartition<T, partition_capacity>);
    }

    Bag(Bag &&) = default;

    ~Bag()
    {
        if (count != 0)
            win::bug("Bag not empty! (" + std::to_string(count) + " items)");
    }

    Bag &operator=(Bag &&) = default;

    template<typename... Ts> T &add(Ts &&...ts)
    {
        if (count == std::numeric_limits<int>::max())
            win::bug("win::Bag: max size");

        unsigned char *spot;

        if (freelist.empty())
        {
            const int partition_number = count / partition_capacity;
            int partition_index = count % partition_capacity;

            auto partition = get_first_partition();
            for (int i = 0; i < partition_number; ++i)
            {
                if (!partition->next)
                    partition->next.reset(new impl::BagPartition<T, partition_capacity>());

                partition = partition->next.get();
            }

            spot = partition->storage[partition_index].item;
        }
        else
        {
            spot = freelist[freelist.size() - 1];
            freelist.pop_back();
        }

        auto *const item = std::launder(new (spot) T(std::forward<Ts>(ts)...));

        ++count;

        return *item;
    }

    void remove(T &object)
    {
        object.~T();

        if (--count == 0)
            freelist.clear(); // this is the last one. wipe the free list
        else
            freelist.push_back(reinterpret_cast<unsigned char *>(&object));
    }

    int size() const { return count; }

private:
    constexpr impl::BagPartition<T, partition_capacity> *get_first_partition()
    {
        if constexpr (first_partition_inline)
            return &first_partition;
        else
            return first_partition.get();
    }

    std::conditional_t<first_partition_inline, impl::BagPartition<T, partition_capacity>, std::unique_ptr<impl::BagPartition<T, partition_capacity>>>
        first_partition;

    std::vector<unsigned char *> freelist;

    int count = 0;
};

}
