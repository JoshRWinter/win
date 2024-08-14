#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <unordered_set>

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
	struct { bool acquired = false; T item; } items[size];
};

struct BlockKey
{
	BlockKey(std::int16_t x, std::int16_t y) : x(x), y(y)
	{}

	bool operator==(BlockKey rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(BlockKey rhs) const { return !operator==(rhs); }

	std::int16_t x;
	std::int16_t y;
};

template<typename T> struct Block
{
	WIN_NO_COPY(Block);
	Block(Block&&) = default;
	Block &operator=(Block&&) = default;
	Block() = default;
	std::vector<T*> items;
	int ghosts = 0;
};

}

template<typename T> class SpatialIndex;
template<typename T> class SpatialIndexIterable;

template<typename T> class SpatialIndexIterator
{
	WIN_NO_COPY(SpatialIndexIterator);

	friend class SpatialIndexIterable<T>;

public:
	SpatialIndexIterator(const SpatialIndex<T> &map, impl::BlockKey key1, impl::BlockKey key2, impl::BlockKey starting_key, std::unordered_set<const T*> *deduper)
		: key1(key1)
		, key2(key2)
		, current_key(starting_key)
		, map(map)
		, deduper(deduper)
	{
		this->map_index = map.index(starting_key);
		this->block_index = 0;
	}

	SpatialIndexIterator(SpatialIndexIterator<T> &&rhs) = default;

	T &operator*() { return *dereference(); }
	bool operator!=(const SpatialIndexIterator<T> &rhs) const { return map_index != rhs.map_index || block_index != rhs.block_index; }

	void operator++()
	{
		++block_index;
		seek_to_next_valid();
	}

private:
	void seek_to_next_valid()
	{
		while (block_index >= block_item_count())
		{
			++current_key.x;

			if (current_key.x > key2.x)
			{
				current_key.x = key1.x;
				++current_key.y;

				map_index = map.index(current_key);
				block_index = 0;

				if (is_exhausted())
					return;
			}
			else
			{
				map_index = map.index(current_key);
				block_index = 0;
			}
		}

		if (is_ghost() || is_dupe())
		{
			++block_index;
			seek_to_next_valid(); // go go gadget tail call recursion?
		}
	}

	bool is_exhausted() const
	{
		return current_key.y > key2.y; // this iterator has completely exhausted its items
	}

	bool is_ghost() const
	{
		return dereference() == NULL;
	}

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

	int block_item_count() const
	{
#ifndef NDEBUG
		return map.map.at(map_index).items.size();
#else
		return map.map[map_index].items.size();
#endif
	}

	T *dereference()
	{
#ifndef NDEBUG
		return map.map.at(map_index).items.at(block_index);
#else
		return map.map[map_index].items[block_index];
#endif
	}

	const T *dereference() const
	{
#ifndef NDEBUG
		return map.map.at(map_index).items.at(block_index);
#else
		return map.map[map_index].items[block_index];
#endif
	}

	int block_index;
	int map_index;
	const impl::BlockKey key1, key2;
	impl::BlockKey current_key;
	const SpatialIndex<T> &map;
	std::unordered_set<const T*> *deduper;
};

template<typename T> class SpatialIndexIterable
{
	WIN_NO_COPY_MOVE(SpatialIndexIterable);

public:
	SpatialIndexIterable(SpatialIndex<T> &map, impl::BlockKey key1, impl::BlockKey key2)
		: key1(key1)
		, key2(key2)
		, corrected_key1(std::max(key1.x, (short)0), std::max(key1.y, (short)0))
		, corrected_key2(std::min(key2.x, (short)(map.map_width - 1)), std::min(key2.y, (short)(map.map_height - 1)))
		, map(map)
		, deduper(corrected_key1 != corrected_key2 ? &map.pool.acquire() : NULL)
	{
		++map.open_iterables;
	}

	~SpatialIndexIterable()
	{
		if (--map.open_iterables == 0)
			map.vacuum();

		if (deduper != NULL)
		{
			deduper->clear();
			map.pool.release(*deduper);
		}
	}

	SpatialIndexIterator<T> begin() const
	{
		if (key1.x >= map.map_width || key1.y >= map.map_height || key2.x < 0 || key2.y < 0)
			return end(); // the sample location that spawned this iterable does not overlap the map at all
		else
		{
			SpatialIndexIterator<T> it(map, corrected_key1, corrected_key2, corrected_key1, deduper);
			it.seek_to_next_valid();
			return std::move(it);
		}
	}

	SpatialIndexIterator<T> end() const
	{
		impl::BlockKey one_past_the_end(corrected_key1.x, corrected_key2.y + 1);
		return SpatialIndexIterator<T>(map, corrected_key1, corrected_key2, one_past_the_end, NULL);
	}

private:
	const impl::BlockKey key1, key2, corrected_key1, corrected_key2;
	std::unordered_set<const T*> *const deduper;
	SpatialIndex<T> &map;
};

struct SpatialIndexLocation
{
	explicit SpatialIndexLocation(float x, float y, float w, float h)
		: x(x), y(y), w(w), h(h)
	{}

	const float x, y, w, h;
};

template<typename T> class SpatialIndex
{
	WIN_NO_COPY_MOVE(SpatialIndex);

	friend class SpatialIndexIterator<T>;
	friend class SpatialIndexIterable<T>;

	static constexpr int vacuum_threshold = 10;

public:
	SpatialIndex()
		: block_size(0.0f)
		, map_left(0.0f)
		, map_right(0.0f)
		, map_bottom(0.0f)
		, map_top(0.0f)
		, map_width(0)
		, map_height(0)
		, open_iterables(0)
	{}

	void reset(float block_size, float map_left, float map_right, float map_bottom, float map_top)
	{
		if (open_iterables > 0)
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

		open_iterables = 0;

		for (auto &item : map)
			item.items.clear();

		map.resize(map_width * map_height);
		for (auto &block: map)
			block.items.reserve(20);

		vacuum_queue.clear();
		vacuum_queue.reserve(map_width * map_height);
	}

	void add(const SpatialIndexLocation &loc, T &id)
	{
		add(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id);
	}

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

	void remove(const SpatialIndexLocation &loc, T &id)
	{
		remove(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id);
	}

	SpatialIndexIterable<T> query(const SpatialIndexLocation &loc)
	{

		const auto key1 = sample(loc.x, loc.y);
		const auto key2 = sample(loc.x + loc.w, loc.y + loc.h);

		return SpatialIndexIterable<T>(*this, key1, key2);
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
				map.at(idx).items.push_back(&id);
#else
				map[idx].items.push_back(&id);
#endif
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
					if (item == &id)
					{
						item = NULL;
						if (++block.ghosts == vacuum_threshold)
							vacuum_queue.push_back(idx);

						found = true;
					}
				}

				if (!found)
					win::bug("Blockmap missing item");
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
				if (*it == NULL)
					it = block.items.erase(it);
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

	int index(impl::BlockKey key) const
	{
		return (key.y * map_width) + key.x;
	}

	float block_size, map_left, map_right, map_bottom, map_top;
	int map_width, map_height;
	std::vector<impl::Block<T>> map;
	impl::SimplePool<std::unordered_set<const T*>, 4> pool;
	int open_iterables;
	std::vector<int> vacuum_queue;
};

}
