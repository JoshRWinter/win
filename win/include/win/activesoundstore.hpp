#ifndef WIN_ACTIVE_SOUND_STORE_HPP
#define WIN_ACTIVE_SOUND_STORE_HPP

#include <vector>

#include <win/win.hpp>
#include <win/pool.hpp>

namespace win
{

enum class SoundPriority
{
	low,
	med,
	high
};

template <typename T> struct ActiveSoundStoreItem : T
{
	template <typename... Ts> ActiveSoundStoreItem(SoundPriority priority, std::uint16_t id, std::uint16_t spot, Ts&&... ts)
		: T(std::forward<Ts>(ts)...)
		, priority(priority)
		, id(id)
		, spot(spot)
	{}

    SoundPriority priority;
	std::uint16_t id;
	int spot;
};

template <typename T> class ActiveSoundStoreIterator
{
public:
	ActiveSoundStoreIterator(win::PoolConstIterator<ActiveSoundStoreItem<T>> iterator)
		: iterator(iterator)
	{}

	ActiveSoundStoreIterator<T> operator++() { ++iterator; return *this; }
	ActiveSoundStoreIterator<T> operator++(int) { ActiveSoundStoreIterator copy = *this; ++iterator; return copy; }
	T &operator*() { return *iterator; }
	T *operator->() { return iterator->operator->(); }
	bool operator==(const ActiveSoundStoreIterator<T> rhs) { return iterator == rhs.iterator; }
	bool operator!=(const ActiveSoundStoreIterator<T> rhs) { return iterator != rhs.iterator; }

private:
	win::PoolConstIterator<ActiveSoundStoreItem<T>> iterator;
};

template <typename T, int count> class ActiveSoundStore
{
	WIN_NO_COPY_MOVE(ActiveSoundStore);

	friend class ActiveSoundStoreIterator<T>;

public:
	ActiveSoundStore()
		: next_id(0)
	{}

	ActiveSoundStoreIterator<T> begin() const
	{
		return ActiveSoundStoreIterator<T>(store.begin());
	}

	ActiveSoundStoreIterator<T> end() const
	{
		return ActiveSoundStoreIterator<T>(store.end());
	}

	T *operator[](std::uint32_t key)
	{
		std::uint16_t spot, id;
		get_key_components(key, spot, id);

		spot_check(spot);

		ActiveSoundStoreItem<T> *item = index[spot];

		if (item == NULL)
			return NULL;

		if (item->id != id)
			return NULL; // original object has been succeeded

		return item;
	}

	template <typename... Ts> std::uint32_t add(SoundPriority priority, Ts&&... args)
	{
		if (store.size() >= count)
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

		ActiveSoundStoreItem<T> &item = store.add(priority, next_id, spot, std::forward<Ts>(args)...);
		++next_id;
		index[spot] = &item;

		return get_key_composite(spot, item.id);
	}

	std::uint32_t find_kickable(SoundPriority priority)
	{
		for (const auto &item : store)
		{
			if (item.priority < priority)
			{
				return item.spot;
			}
		}

		return -1;
	}

	ActiveSoundStoreIterator<T> remove(ActiveSoundStoreIterator<T> it)
	{
		return remove(*it);
	}

	void remove(std::uint32_t key)
	{
		std::uint16_t spot, id;
		get_key_components(key, spot, id);

		spot_check(spot);

		const auto &item = *index[spot];
		if (item.id != id)
			return; // original object has been succeeded

		remove(*index[spot]);
	}

private:

	ActiveSoundStoreIterator<T> remove(T &t)
	{
		ActiveSoundStoreItem<T> &item = *static_cast<ActiveSoundStoreItem<T>*>(&t);

		if (store.count() == 1)
			index_free_list.clear(); // last item, wipe the list
		else
			index_free_list.push_back(item.spot);

		index[item.spot] = NULL;

		return ActiveSoundStoreIterator<T>(store.remove(item));
	}

    static void get_key_components(const std::uint32_t key, std::uint16_t &spot, std::uint16_t &id)
	{
	    spot = key & 0xffff;
		id = key >> 16;
	}

	static std::uint32_t get_key_composite(const std::uint16_t spot, const std::uint16_t id)
	{
		std::uint32_t key = id;
		key <<= 16;
		key |= spot;

		return key;
	}

	static void spot_check(std::uint16_t spot)
	{
		if (spot < 0 || spot >= count)
			win::bug("Spot out of range");
	}

	std::uint16_t next_id;
	win::Pool<ActiveSoundStoreItem<T>> store;
	ActiveSoundStoreItem<T> *index[count];
	std::vector<std::uint32_t> index_free_list;
};

}

#endif
