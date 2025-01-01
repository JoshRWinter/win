#pragma once

#include <type_traits>
#include <memory>
#include <vector>
#include <limits>

#include <win/Win.hpp>

namespace win
{

template <typename T, int initial_capacity> class Pool;

namespace impl
{

template <typename T> struct PoolNode : T
{
	template <typename... Ts> PoolNode(Ts&&... ts)
		: T(std::forward<Ts>(ts)...)
	{}

	void *spot;
	PoolNode<T> *prev;
	PoolNode<T> *next;
};

template <typename T, int capacity> struct PoolPartition
{
	WIN_NO_COPY_MOVE(PoolPartition);

	PoolPartition() = default;

	std::aligned_storage_t<sizeof(PoolNode<T>), alignof(PoolNode<T>)> storage[capacity];
	std::unique_ptr<PoolPartition<T, capacity>> next;
};


template <typename T, int initial_capacity> class PoolIterator
{
	friend class Pool<T, initial_capacity>;
public:
	explicit PoolIterator(impl::PoolNode<T> *node)
		: node(node) {}

	PoolIterator<T, initial_capacity> operator++() { node = node->next; return *this; }
	PoolIterator<T, initial_capacity> operator++(int) { PoolIterator<T, initial_capacity> copy = *this; node = node->next; return copy; }
	T &operator*() { return *node; }
	T *operator->() { return node; }
	bool operator==(const PoolIterator<T, initial_capacity> rhs) const { return node == rhs.node; }
	bool operator!=(const PoolIterator<T, initial_capacity> rhs) const { return node != rhs.node; }

private:
	impl::PoolNode<T> *node;
};

template <typename T, int initial_capacity> class PoolConstIterator
{
	friend class Pool<T, initial_capacity>;
public:
	explicit PoolConstIterator(const impl::PoolNode<T> *node)
		: node(node) {}

	PoolConstIterator<T, initial_capacity> operator++() { node = node->next; return *this; }
	PoolConstIterator<T, initial_capacity> operator++(int) { PoolConstIterator<T, initial_capacity> copy = *this; node = node->next; return copy; }
	const T &operator*() const { return *node; }
	const T *operator->() const { return node; }
	bool operator==(const PoolConstIterator<T, initial_capacity> rhs) const { return node == rhs.node; }
	bool operator!=(const PoolConstIterator<T, initial_capacity> rhs) const { return node != rhs.node; }

private:
	const impl::PoolNode<T> *node;
};

}

template <typename T, int initial_capacity = 100> class Pool
{
	struct Empty {};

	WIN_NO_COPY_MOVE(Pool);

public:
	typedef impl::PoolIterator<T, initial_capacity> Iterator;
	typedef impl::PoolConstIterator<T, initial_capacity> ConstIterator;

	Pool()
		: count(0)
		, head(NULL)
		, tail(NULL)
	{
		static_assert(initial_capacity > 0, "Capacity must be greater than zero.");

		if constexpr (use_heap_storage())
			first_partition_heap_ptr.reset(new impl::PoolPartition<T, initial_capacity>);
	}

	~Pool()
	{
		clear();
	}

	Iterator begin() { return Iterator(head); }
	Iterator end() { return Iterator(NULL); }
	ConstIterator begin() const { return ConstIterator(head); }
	ConstIterator end() const { return ConstIterator(NULL); }

	int size() const { return count; }

	void clear()
	{
		Iterator it = begin();
		while (it != end())
			it = remove(it);

		count = 0;
	}

	template <typename... Ts> T &add(Ts&&... ts)
	{
		if (count == std::numeric_limits<int>::max())
			win::bug("win::Pool: max size");

		std::aligned_storage_t<sizeof(impl::PoolNode<T>), alignof(impl::PoolNode<T>)> *const spot = find_spot();

		impl::PoolNode<T> *node = new (spot) impl::PoolNode<T>(std::forward<Ts>(ts)...);
		node->spot = spot;

		node->next = NULL;
		node->prev = tail;

		if (tail != NULL)
			tail->next = node;
		else
			head = node;

		tail = node;

		++count;
		return *node;
	}

	void remove(T &object)
	{
		erase(object);
	}

	Iterator remove(Iterator it)
	{
		return erase(*it.node);
	}

private:
	Iterator erase(T &object)
	{
		impl::PoolNode<T> *node = static_cast<impl::PoolNode<T>*>(&object);

		if (node->prev != NULL)
			node->prev->next = node->next;
		else
			head = node->next;

		if (node->next != NULL)
			node->next->prev = node->prev;
		else
			tail = node->prev;

		if (--count == 0)
			freelist.clear(); // this is the last one. wipe the free list
		else
			freelist.push_back(static_cast<std::aligned_storage_t<sizeof(impl::PoolNode<T>), alignof(impl::PoolNode<T>)>*>(node->spot));

		impl::PoolNode<T> *next = node->next;
#ifndef NDEBUG
		node->next = NULL;
#endif
		node->~PoolNode();
		return Iterator(next);
	}

	std::aligned_storage_t<sizeof(impl::PoolNode<T>), alignof(impl::PoolNode<T>)> *find_spot()
	{
		if (!freelist.empty())
		{
			std::aligned_storage_t<sizeof(impl::PoolNode<T>), alignof(impl::PoolNode<T>)> *spot = freelist[freelist.size() - 1];
			freelist.pop_back();
			return spot;
		}

		const int partition_outer_offset = count / initial_capacity;
		int partition_inner_offset = count % initial_capacity;

		impl::PoolPartition<T, initial_capacity> *partition = get_first_partition();
		for (int i = 0; i < partition_outer_offset; ++i)
		{
			if (!partition->next)
				partition->next.reset(new impl::PoolPartition<T, initial_capacity>());

			partition = partition->next.get();
		}

		return partition->storage + partition_inner_offset;
	}

	// use heap storage for first head) partition?
	static constexpr bool use_heap_storage()
	{
		return sizeof(T) * initial_capacity > 500;
	}

	constexpr impl::PoolPartition<T, initial_capacity> *get_first_partition()
	{
		if constexpr (use_heap_storage())
			return first_partition_heap_ptr.get();
		else
			return &first_partition_storage;
	}

	int count;
	std::vector<std::aligned_storage_t<sizeof(impl::PoolNode<T>), alignof(impl::PoolNode<T>)>*> freelist;
	[[no_unique_address]] std::conditional_t<use_heap_storage(), std::unique_ptr<impl::PoolPartition<T, initial_capacity>>, Empty> first_partition_heap_ptr;
	[[no_unique_address]] std::conditional_t<!use_heap_storage(), impl::PoolPartition<T, initial_capacity>, Empty> first_partition_storage;
	impl::PoolNode<T> *head;
	impl::PoolNode<T> *tail;
};

}
