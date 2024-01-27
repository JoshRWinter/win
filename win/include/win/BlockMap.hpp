#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <unordered_set>

#include <win/Win.hpp>

namespace win
{

template<typename T> class BlockMap;

namespace impl
{

template<typename T, int size> class BlockMapDeduperPool
{
	WIN_NO_COPY_MOVE(BlockMapDeduperPool);

public:
	BlockMapDeduperPool() = default;

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

		win::bug("BlockMapDeduperPool: no items. max collision check depth reached.");
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

		win::bug("BlockMapDeduperPool: invalid release");
	}

private:
	struct { bool acquired = false; T item; } items[size];
};

struct BlockKey
{
	BlockKey(std::uint16_t x, std::uint16_t y) : x(x), y(y)
	{}

	bool operator==(BlockKey rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(BlockKey rhs) const { return x != rhs.x || y != rhs.y; }

	std::uint16_t x;
	std::uint16_t y;
};

template<typename T> class BlockMapIterable;

template<typename T> class BlockMapIterator
{
	WIN_NO_COPY_MOVE(BlockMapIterator);
	friend class BlockMapIterable<T>;

public:
	BlockMapIterator(const BlockMap<T> &map, BlockKey key1, BlockKey key2, std::unordered_set<T*> *deduper, bool end = false)
		: x(key1.x)
		, y(key1.y)
		, key1(key1)
		, key2(key2)
		, map(map)
		, deduper(deduper)
	{
		if (end)
		{
			map_index = map.index(key2);
			block_index = map.map[map_index].items.size();
		}
		else
		{
			map_index = map.index(key1);
			block_index = -1;

			next();
		}
	}

	T &operator*()
	{
#ifndef NDEBUG
		return *map.map.at(map_index).items.at(block_index);
#else
		return *map.map[map_index].items[block_index];
#endif
	}

	void operator++() { next(); }

	//bool operator==(const BlockMapIterator<T> &rhs) const { return end == rhs.end; }//return block_index == rhs.block_index && map_index == rhs.map_index; }
	bool operator!=(const BlockMapIterator<T> &rhs) const { return block_index != rhs.block_index || map_index != rhs.map_index; }

private:
	void next()
	{
		bool map_exhausted = is_map_exhausted();
		bool block_exhausted;

		do
		{
			++block_index;
			block_exhausted = is_block_exhausted();

			while (block_exhausted && !map_exhausted)
			{
				block_index = 0;
				++x;
				if (x > key2.x)
				{
					x = key1.x;
					++y;
				}

				map_index = map.index(BlockKey(x, y));

				map_exhausted = is_map_exhausted();
				block_exhausted = is_block_exhausted();
			}

		} while (!block_exhausted && (is_ghost() || is_dupe()));
	}

	bool is_ghost() const
	{
#ifndef NDEBUG
		return map.map.at(map_index).items.at(block_index) == NULL;
#else
		return map.map[map_index].items[block_index] == NULL;
#endif
	}

	bool is_dupe() const
	{
		if (deduper != NULL)
		{
#ifndef NDEBUG
			T *value = map.map.at(map_index).items.at(block_index);
#else
			T *value = map.map[map_index].items[block_index];
#endif

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

	bool is_block_exhausted() const
	{
#ifndef NDEBUG
		return block_index == map.map.at(map_index).items.size();
#else
		return block_index == map.map[map_index].items.size();
#endif
	}

	bool is_map_exhausted() const { return x == key2.x && y == key2.y; }

	int block_index;
	int map_index;
	int x, y;
	const BlockKey key1, key2;
	const BlockMap<T> &map;
	std::unordered_set<T*> *deduper;
};

template<typename T> class BlockMapIterable
{
	WIN_NO_COPY_MOVE(BlockMapIterable);

public:
	BlockMapIterable(BlockMap<T> &map, BlockKey key1, BlockKey key2)
		: key1(key1)
		, key2(key2)
		, map(map)
		, deduper(key1 != key2 ? &map.pool.acquire() : NULL)
	{
		++map.open_iterables;
	}

	~BlockMapIterable()
	{
		if (--map.open_iterables == 0)
			map.vacuum();

		if (deduper != NULL)
		{
			deduper->clear();
			map.pool.release(*deduper);
		}
	}

	BlockMapIterator<T> begin() const { return BlockMapIterator<T>(map, key1, key2, deduper); }
	BlockMapIterator<T> end() const { return BlockMapIterator<T>(map, key1, key2, NULL, true); }

private:
	const BlockKey key1, key2;
	std::unordered_set<T*> *const deduper;
	BlockMap<T> &map;
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

struct BlockMapLocation
{
	explicit BlockMapLocation(float x, float y, float w, float h)
		: x(x), y(y), w(w), h(h)
	{}

	const float x, y, w, h;
};

template<typename T> class BlockMap
{
	WIN_NO_COPY_MOVE(BlockMap);

	friend class impl::BlockMapIterator<T>;
	friend class impl::BlockMapIterable<T>;

	static constexpr int vacuum_threshold = 10;

public:
	BlockMap()
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
		this->block_size = block_size;
		this->map_left = map_left;
		this->map_right = map_right;
		this->map_bottom = map_bottom;
		this->map_top = map_top;
		this->map_width = std::ceil(map_right - map_left) / block_size;
		this->map_height = std::ceil(map_top - map_bottom) / block_size;

		open_iterables = 0;
		map.clear();
		map.resize(map_width * map_height);
		for (auto &block: map)
			block.items.reserve(20);

		vacuum_queue.reserve(map_width * map_height);
	}

	void add(const BlockMapLocation &loc, T &id)
	{
		insert_or_delete<true>(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id);
	}

	void move(const BlockMapLocation &old_loc, const BlockMapLocation &new_loc, T &id)
	{
		const auto old_key1 = sample(old_loc.x, old_loc.y);
		const auto old_key2 = sample(old_loc.x + old_loc.w, old_loc.y + old_loc.h);

		const auto new_key1 = sample(new_loc.x, new_loc.y);
		const auto new_key2 = sample(new_loc.x + new_loc.w, new_loc.y + new_loc.h);

		if (old_key1 == new_key1 && old_key2 == new_key2)
			return; // object is still in the same block. no update needed

		insert_or_delete<false>(old_key1, old_key2, id);
		insert_or_delete<true>(new_key1, new_key2, id);
	}

	void remove(const BlockMapLocation &loc, T &id)
	{
		insert_or_delete<false>(sample(loc.x, loc.y), sample(loc.x + loc.w, loc.y + loc.h), id);
	}

	impl::BlockMapIterable<T> iterate(const BlockMapLocation &loc)
	{

		const auto key1 = sample(loc.x, loc.y);
		const auto key2 = sample(loc.x + loc.w, loc.y + loc.h);

		return impl::BlockMapIterable<T>(*this, key1, key2);
	}

private:
	template<bool add> void insert_or_delete(impl::BlockKey a, impl::BlockKey b, T &id)
	{
		for (auto x = a.x; x <= b.x; ++x)
		{
			for (auto y = a.y; y <= b.y; ++y)
			{
				const auto idx = index(impl::BlockKey(x, y));

				if constexpr (add)
				{
#ifndef NDEBUG
					map.at(idx).items.push_back(&id);
#else
					map[idx].items.push_back(&id);
#endif
				}
				else
				{
#ifndef NDEBUG
					auto &block = map.at(idx);
#else
					auto &block = map[idx];
#endif

					for (auto &item : block.items)
					{
						if (item == &id)
						{
							item = NULL;
							if (++block.ghosts == vacuum_threshold)
								vacuum_queue.push_back(idx);

							return;
						}
					}

					win::bug("Blockmap missing item");
				}
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
#ifndef NDEBUG
		if (x < map_left || x > map_right)
			win::bug("BlockMap: x out of bounds (x=" + std::to_string(x) + ", map_left=" + std::to_string(map_left) + ", map_right=" + std::to_string(map_right) + ")");
		if (y < map_bottom || y > map_top)
			win::bug("BlockMap: y out of bounds (y=" + std::to_string(y) + ", map_bottom=" + std::to_string(map_bottom) + ", map_top=" + std::to_string(map_top) + ")");
#endif

		const std::uint16_t blockx = std::floor((x - map_left) / block_size);
		const std::uint16_t blocky = std::floor((y - map_bottom) / block_size);

		return impl::BlockKey(blockx, blocky);
	}

	int index(impl::BlockKey key) const
	{
		return (key.y * map_width) + key.x;
	}

	float block_size, map_left, map_right, map_bottom, map_top;
	int map_width, map_height;
	std::vector<impl::Block<T>> map;
	impl::BlockMapDeduperPool<std::unordered_set<T*>, 4> pool;
	int open_iterables;
	std::vector<int> vacuum_queue;
};

}
