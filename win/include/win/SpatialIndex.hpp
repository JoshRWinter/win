#pragma once

#include <cmath>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include <win/Pool.hpp>
#include <win/Win.hpp>

namespace win
{

namespace impl
{

template<typename T, int size> class SimplePool
{
    WIN_NO_COPY_MOVE(SimplePool);

public:
    SimplePool() = default;

    T &acquire()
    {
        for (int i = 0; i < size; ++i)
        {
            if (!items[i].acquired)
            {
                items[i].acquired = true;
                return items[i].item;
            }
        }

        win::bug("SimplePool: no items. max collision check depth reached.");
    }

    void release(T &item)
    {
        for (int i = 0; i < size; ++i)
        {
            if (&item == &items[i].item)
            {
                items[i].acquired = false;
                return;
            }
        }

        win::bug("SimplePool: invalid release");
    }

private:
    struct
    {
        bool acquired = false;
        T item;
    } items[size];
};

struct BlockKey
{
    BlockKey(std::int16_t x, std::int16_t y)
        : x(x)
        , y(y)
    {
    }

    bool operator==(BlockKey rhs) const { return x == rhs.x && y == rhs.y; }

    bool operator!=(BlockKey rhs) const { return !operator==(rhs); }

    std::int16_t x;
    std::int16_t y;
};

template<typename T> struct Block
{
    WIN_NO_COPY(Block);

    static constexpr int bag_part_size = 50;
    typedef win::Pool<win::PoolBox<T *>, bag_part_size, false, true> PoolType;

    explicit Block(win::Bag<win::PoolNode<win::PoolBox<T *>>, bag_part_size, false> &bag)
        : items(bag)
    {
    }

    Block(Block &&) = default;
    Block &operator=(Block &&) = default;

    PoolType items;
    int ghosts = 0;
};

}

template<typename T> class SpatialIndex;

template<typename T> class SpatialIndexIterator
{
    friend class SpatialIndex<T>;

public:
    SpatialIndexIterator(const SpatialIndex<T> &map,
                         std::unordered_set<const T *> *deduper,
                         typename impl::Block<T>::PoolType::ConstIterator block_iterator,
                         impl::BlockKey key1,
                         impl::BlockKey key2,
                         impl::BlockKey starting_key)
        : map(map)
        , deduper(deduper)
        , block_iterator(block_iterator)
        , key1(key1)
        , key2(key2)
        , current_key(starting_key)
        , map_index(map.index(starting_key))
    {
    }

    T &operator*() { return *dereference(); }

    bool operator!=(const SpatialIndexIterator<T> &rhs) const { return map_index != rhs.map_index || block_iterator != rhs.block_iterator; }

    void operator++()
    {
        ++block_iterator;
        seek_to_next_valid();
    }

private:
    void seek_to_next_valid()
    {
        while (true)
        {
            while (block_iterator == get_block().items.end())
            {
                ++current_key.x;

                if (current_key.x > key2.x)
                {
                    current_key.x = key1.x;
                    ++current_key.y;

                    map_index = map.index(current_key);

                    if (is_exhausted()) // This entire index query is kaput
                        return;
                    else
                        block_iterator = get_block().items.begin();
                }
                else
                {
                    map_index = map.index(current_key);
                    block_iterator = get_block().items.begin();
                }
            }

            if (is_ghost() || is_dupe())
                ++block_iterator;
            else
                return;
        }
    }

    bool is_exhausted() const
    {
        return current_key.y > key2.y; // this iterator has completely exhausted its items
    }

    bool is_ghost() const { return dereference() == NULL; }

    bool is_dupe() const
    {
        if (deduper != NULL)
        {
            const auto *value = dereference();

            if (deduper->count(value) == 1)
            {
                return true;
            }
            else
            {
                deduper->insert(value);
                return false;
            }
        }
        else
            return false;
    }

    const win::impl::Block<T> &get_block() const
    {
#ifndef NDEBUG
        return map.map.at(map_index);
#else
        return map.map[map_index];
#endif
    }

    T *dereference() const { return block_iterator->inner; }

    const SpatialIndex<T> &map;
    std::unordered_set<const T *> *deduper;
    typename impl::Block<T>::PoolType::ConstIterator block_iterator;
    const impl::BlockKey key1;
    const impl::BlockKey key2;
    impl::BlockKey current_key;
    int map_index;
};

template<typename T> class SpatialIndexQuery
{
    WIN_NO_COPY_MOVE(SpatialIndexQuery);

public:
    SpatialIndexQuery(SpatialIndex<T> &map, std::unordered_set<const T *> *deduper, SpatialIndexIterator<T> it_begin, SpatialIndexIterator<T> it_end)
        : map(map)
        , deduper(deduper)
        , it_begin(it_begin)
        , it_end(it_end)
    {
        ++map.open_queries;
    }

    ~SpatialIndexQuery()
    {
        if (deduper != NULL)
        {
            deduper->clear();
            map.dedupers.release(*deduper);
        }

        if (--map.open_queries == 0)
            map.vacuum();
    }

    SpatialIndexIterator<T> begin() const { return it_begin; }

    SpatialIndexIterator<T> end() const { return it_end; }

private:
    SpatialIndex<T> &map;
    std::unordered_set<const T *> *deduper;
    SpatialIndexIterator<T> it_begin;
    SpatialIndexIterator<T> it_end;
};

struct SpatialIndexLocation
{
    explicit SpatialIndexLocation(float x, float y, float w, float h)
        : x(x)
        , y(y)
        , w(w)
        , h(h)
    {
    }

    const float x, y, w, h;
};

template<typename T> class SpatialIndex
{
    WIN_NO_COPY_MOVE(SpatialIndex);

    friend class SpatialIndexIterator<T>;
    friend class SpatialIndexQuery<T>;

    static constexpr int vacuum_threshold = 10;

public:
    SpatialIndex() = default;

    void reset(float block_size, float map_left, float map_right, float map_bottom, float map_top)
    {
        if (open_queries != 0)
            win::bug("SpatialIndex: Can't reset while iterating!");

        this->block_size = block_size;
        this->map_left = map_left;
        this->map_right = map_right;
        this->map_bottom = map_bottom;
        this->map_top = map_top;
        this->map_width = std::ceil((map_right - map_left) / block_size);
        this->map_height = std::ceil((map_top - map_bottom) / block_size);

        if (this->map_width < 1 || this->map_height < 1)
            win::bug("SpatialIndex: to small");

        if (this->map_width > std::numeric_limits<std::int16_t>::max() || this->map_height > std::numeric_limits<std::int16_t>::max())
            win::bug("SpatialIndex: exceeds integer limits.");

        map.clear();
        map.reserve(map_width * map_height);

        for (int i = 0; i < map_width * map_height; ++i)
            map.emplace_back(bag);

        vacuum_queue.clear();
        vacuum_queue.reserve(map_width * map_height);
    }

    void add(const SpatialIndexLocation &loc, T &id) { add(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id); }

    void move(const SpatialIndexLocation &old_loc, const SpatialIndexLocation &new_loc, T &id)
    {
        const auto old_key1 = sample(old_loc.x, old_loc.y);
        const auto old_key2 = sample(old_loc.x + old_loc.w, old_loc.y + old_loc.h);

        const auto new_key1 = sample(new_loc.x, new_loc.y);
        const auto new_key2 = sample(new_loc.x + new_loc.w, new_loc.y + new_loc.h);

        if (old_key1 == new_key1 && old_key2 == new_key2)
            return; // object is still in the same location. no update needed

        remove(old_key1, old_key2, id);
        add(new_key1, new_key2, id);
    }

    void remove(const SpatialIndexLocation &loc, T &id) { remove(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id); }

    SpatialIndexQuery<T> query(const SpatialIndexLocation &loc)
    {
        const auto key1 = sample(loc.x, loc.y);
        const auto key2 = sample(loc.x + loc.w, loc.y + loc.h);

        if (key1.x > key2.x || key1.y > key2.y)
            win::bug("SpatialIndex: location has negative area?");

        // detect the location being completely outside the index
        const bool outside = key1.x >= map_width || key1.y >= map_height || key2.x < 0 || key2.y < 0;

        // fit the keys to the actual bounds of the index
        const auto corrected_key1_x = std::min(std::max((std::int16_t)0, key1.x), (std::int16_t)(map_width - 1));
        const auto corrected_key1_y = std::min(std::max((std::int16_t)0, key1.y), (std::int16_t)(map_height - 1));
        const auto corrected_key2_x = std::min(std::max((std::int16_t)0, key2.x), (std::int16_t)(map_width - 1));
        const auto corrected_key2_y = std::min(std::max((std::int16_t)0, key2.y), (std::int16_t)(map_height - 1));

        const impl::BlockKey corrected_key1(corrected_key1_x, corrected_key1_y);
        const impl::BlockKey corrected_key2(corrected_key2_x, corrected_key2_y);

        const impl::BlockKey one_past_the_end(corrected_key1.x, (std::int16_t)(corrected_key2.y + 1));
        SpatialIndexIterator<T> end(*this, NULL, typename impl::Block<T>::PoolType::ConstIterator(NULL), corrected_key1, corrected_key2, one_past_the_end);

        if (outside)
        {
            return SpatialIndexQuery<T>(*this, NULL, end, end);
        }
        else
        {
            auto deduper = key1 == key2 ? NULL : &dedupers.acquire();

#ifdef NDEBUG
            const auto &starting_block = map[index(corrected_key1)];
#else
            const auto &starting_block = map.at(index(corrected_key1));
#endif

            SpatialIndexIterator<T> begin(*this, deduper, starting_block.items.begin(), corrected_key1, corrected_key2, corrected_key1);
            begin.seek_to_next_valid();

            return SpatialIndexQuery<T>(*this, deduper, begin, end);
        }
    }

private:
    void add(impl::BlockKey a, impl::BlockKey b, T &id)
    {
        for (auto x = std::max(a.x, (short)0); x <= std::min(b.x, (short)(map_width - 1)); ++x)
        {
            for (auto y = std::max(a.y, (short)0); y <= std::min(b.y, (short)(map_height - 1)); ++y)
            {
                const auto idx = index(impl::BlockKey(x, y));

#ifndef NDEBUG
                auto &block = map.at(idx);
#else
                auto &block = map[idx];
#endif

                block.items.add(&id);
            }
        }
    }

    void remove(impl::BlockKey a, impl::BlockKey b, T &id)
    {
        for (auto x = std::max(a.x, (short)0); x <= std::min(b.x, (short)(map_width - 1)); ++x)
        {
            for (auto y = std::max(a.y, (short)0); y <= std::min(b.y, (short)(map_height - 1)); ++y)
            {
                const auto idx = index(impl::BlockKey(x, y));

#ifndef NDEBUG
                auto &block = map.at(idx);
#else
                auto &block = map[idx];
#endif

                bool found = false;
                for (auto &item : block.items)
                {
                    if (item.inner == &id)
                    {
                        item.inner = NULL;
                        if (++block.ghosts == vacuum_threshold)
                            vacuum_queue.push_back(idx);

                        found = true;
                    }
                }

                if (!found)
                    win::bug("SpatialIndex: missing item");
            }
        }
    }

    void vacuum()
    {
        for (auto idx : vacuum_queue)
        {
            auto &block = map[idx];
            block.ghosts = 0;
            for (auto it = block.items.begin(); it != block.items.end();)
            {
                if (it->inner == NULL)
                    it = block.items.remove(it);
                else
                    ++it;
            }
        }

        vacuum_queue.clear();
    }

    impl::BlockKey sample(float x, float y) const
    {
        const std::int16_t blockx = std::floor((x - map_left) / block_size);
        const std::int16_t blocky = std::floor((y - map_bottom) / block_size);

        return impl::BlockKey(blockx, blocky);
    }

    int index(impl::BlockKey key) const { return (key.y * map_width) + key.x; }

    float block_size = 0.0f;

    float map_left = 0.0f;
    float map_right = 0.0f;
    float map_bottom = 0.0f;
    float map_top = 0.0f;

    int map_width = 0;
    int map_height = 0;

    win::Bag<win::PoolNode<win::PoolBox<T *>>, impl::Block<T>::bag_part_size, false> bag;
    std::vector<impl::Block<T>> map;
    impl::SimplePool<std::unordered_set<const T *>, 4> dedupers;

    int open_queries = 0;

    std::vector<int> vacuum_queue;
};

}
