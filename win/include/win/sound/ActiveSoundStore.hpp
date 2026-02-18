#pragma once

#include <vector>

#include <cstring>

#include <win/Win.hpp>
#include <win/Pool.hpp>

namespace win
{

template <typename T> struct ActiveSoundStoreItem : T
{
	template <typename... Ts> ActiveSoundStoreItem(std::uint16_t id, std::uint16_t spot, Ts&&... ts)
		: T(std::forward<Ts>(ts)...)
		, id(id)
		, spot(spot)
	{}

	std::uint16_t id;
	int spot;
};

template <typename T, int capacity> class ActiveSoundStore;
template <typename T, int capacity> class ActiveSoundStoreIterator
{
	friend class ActiveSoundStore<T, capacity>;

public:
	ActiveSoundStoreIterator(typename win::Pool<ActiveSoundStoreItem<T>, capacity, true>::Iterator iterator)
		: iterator(iterator)
	{}

	ActiveSoundStoreIterator<T, capacity> operator++() { ++iterator; return *this; }
	ActiveSoundStoreIterator<T, capacity> operator++(int) { ActiveSoundStoreIterator<T, capacity> copy = *this; ++iterator; return copy; }
	T &operator*() { return *iterator; }
	T *operator->() { return iterator.operator->(); }
	bool operator==(const ActiveSoundStoreIterator<T, capacity> rhs) { return iterator == rhs.iterator; }
	bool operator!=(const ActiveSoundStoreIterator<T, capacity> rhs) { return iterator != rhs.iterator; }

private:
	typename win::Pool<ActiveSoundStoreItem<T>, capacity, true>::Iterator iterator;
};

template <typename T, int capacity> class ActiveSoundStore
{
	WIN_NO_COPY_MOVE(ActiveSoundStore);
	static_assert(capacity <= 32, "Reevaluate first_partition_inline for capacities other than 32");

public:
	typedef class ActiveSoundStoreIterator<T, capacity> Iterator;

	static constexpr int maxsize = capacity;

	ActiveSoundStore()
		: next_id(0)
	{
		static_assert(capacity > 0 && capacity <= std::numeric_limits<std::uint16_t>::max(), "must be between 1 and uint16 max");
		memset(index, 0, sizeof(index));
	}

	ActiveSoundStoreIterator<T, capacity> begin()
	{
		return ActiveSoundStoreIterator<T, capacity>(store.begin());
	}

	ActiveSoundStoreIterator<T, capacity> end()
	{
		return ActiveSoundStoreIterator<T, capacity>(store.end());
	}

	int size() const { return store.size(); }

	T *operator[](std::uint32_t key)
	{
		if (key == -1)
			return NULL;

		std::uint16_t spot, id;
		get_key_components(key, id, spot);

		spot_check(spot);

		ActiveSoundStoreItem<T> *item = index[spot];

		if (item == NULL)
			return NULL; // object doesn't exist anymore

		if (item->id != id)
			return NULL; // original object has been succeeded

		return item;
	}

	template <typename... Ts> std::uint32_t add(Ts&&... args)
	{
		if (store.size() >= capacity)
			return -1;

		std::uint16_t spot;
		if (index_free_list.size() == 0)
		{
			spot = store.size();
		}
		else
		{
			spot = index_free_list[index_free_list.size() - 1];
			index_free_list.erase(index_free_list.end() - 1);
		}

		ActiveSoundStoreItem<T> &item = store.add(next_id, spot, std::forward<Ts>(args)...);
		++next_id;
		index[spot] = &item;

		return get_key_composite(item.id, spot);
	}

	ActiveSoundStoreIterator<T, capacity> remove(ActiveSoundStoreIterator<T, capacity> it)
	{
		if (store.size() == 1)
			index_free_list.clear(); // last item, wipe the list
		else
			index_free_list.push_back(it.iterator->spot);

		index[it.iterator->spot] = NULL;
		return store.remove(it.iterator);
	}

	void remove(std::uint32_t key)
	{
		if (key == -1)
			return;

		std::uint16_t spot, id;
		get_key_components(key, id, spot);

		spot_check(spot);

		if (index[spot] == NULL)
			return; // no such object

		ActiveSoundStoreItem<T> &item = *index[spot];
		if (item.id != id)
			return; // original object has been succeeded

		if (store.size() == 1)
			index_free_list.clear(); // last item, wipe the list
		else
			index_free_list.push_back(item.spot);

		index[item.spot] = NULL;
		store.remove(item);
	}

private:
	static void get_key_components(const std::uint32_t key, std::uint16_t &id, std::uint16_t &spot)
	{
		id = key >> 16;
		spot = key & 0xffff;
	}

	static std::uint32_t get_key_composite(const std::uint16_t id, const std::uint16_t spot)
	{
		std::uint32_t key = id;
		key <<= 16;
		key |= spot;

		return key;
	}

	static void spot_check(std::uint16_t spot)
	{
		if (spot < 0 || spot >= capacity)
			win::bug("Spot out of range");
	}

	std::uint16_t next_id;
	win::Pool<ActiveSoundStoreItem<T>, capacity, true> store;
	ActiveSoundStoreItem<T> *index[capacity];
	std::vector<std::uint32_t> index_free_list;
};

}
