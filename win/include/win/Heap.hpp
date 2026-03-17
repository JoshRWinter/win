#pragma once

#include <memory>
#include <vector>

#include <win/Win.hpp>

namespace win
{

namespace impl
{

template<typename T> struct HeapItem
{
    alignas(alignof(T)) unsigned char item[sizeof(T)];
};

template<typename T, int capacity> struct HeapPartition
{
    WIN_NO_COPY_MOVE(HeapPartition);

    HeapPartition() = default;

    std::unique_ptr<HeapPartition<T, capacity>> next;
    HeapItem<T> storage[capacity];
};

}

template<typename T, int partition_capacity, bool first_partition_inline> class Heap
{
    WIN_NO_COPY_MOVE(Heap);

public:
    Heap()
    {
        static_assert(partition_capacity > 0, "Capacity must be greater than zero.");

        if constexpr (!first_partition_inline)
            first_partition.reset(new impl::HeapPartition<T, partition_capacity>);
    }

    ~Heap()
    {
        if (count != 0)
            win::bug("Heap not empty! (" + std::to_string(count) + " items)");
    }

    template<typename... Ts> T &add(Ts &&...ts)
    {
        if (count == std::numeric_limits<int>::max())
            win::bug("win::Heap: max size");

        unsigned char *spot;

        if (freelist.empty())
        {
            const int partition_number = count / partition_capacity;
            int partition_index = count % partition_capacity;

            auto partition = get_first_partition();
            for (int i = 0; i < partition_number; ++i)
            {
                if (!partition->next)
                    partition->next.reset(new impl::HeapPartition<T, partition_capacity>());

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
    constexpr impl::HeapPartition<T, partition_capacity> *get_first_partition()
    {
        if constexpr (first_partition_inline)
            return &first_partition;
        else
            return first_partition.get();
    }

    std::conditional_t<first_partition_inline, impl::HeapPartition<T, partition_capacity>, std::unique_ptr<impl::HeapPartition<T, partition_capacity>>>
        first_partition;

    std::vector<unsigned char *> freelist;

    int count = 0;
};

}
