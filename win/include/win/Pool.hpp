#pragma once

#include <memory>
#include <vector>

#include <win/Win.hpp>

namespace win
{

namespace impl
{

template<typename T> struct PoolNodeStorage
{
	alignas(alignof(T)) unsigned char item[sizeof(T)];
};

template<typename T> struct PoolNode : T
{
	template<typename... Ts> PoolNode(Ts &&...ts)
		: T(std::forward<Ts>(ts)...)
	{
	}

	PoolNode<T> *prev;
	PoolNode<T> *next;
};

template<typename T, int capacity> struct PoolPartition
{
	WIN_NO_COPY_MOVE(PoolPartition);

	PoolPartition() = default;

	std::unique_ptr<PoolPartition<T, capacity>> next;
	PoolNodeStorage<PoolNode<T>> storage[capacity];
};

template<typename T> class PoolIterator
{
	friend class Pool;

public:
	explicit PoolIterator(PoolNode<T> *node)
		: node(node)
	{
	}

	PoolIterator<T> operator++()
	{
		node = node->next;
		return *this;
	}

	PoolIterator<T> operator++(int)
	{
		PoolIterator<T> copy(node);
		node = node->next;
		return copy;
	}

	T &operator*() { return *node; }

	T *operator->() { return node; }

	bool operator==(const PoolIterator<T> rhs) const { return node == rhs.node; }

	bool operator!=(const PoolIterator<T> rhs) const { return node != rhs.node; }

private:
	PoolNode<T> *node;
};

template<typename T> class PoolConstIterator
{
public:
	explicit PoolConstIterator(const PoolNode<T> *node)
		: node(node)
	{
	}

	PoolConstIterator<T> operator++()
	{
		node = node->next;
		return *this;
	}

	PoolConstIterator<T> operator++(int)
	{
		PoolConstIterator<T> copy(node);
		node = node->next;
		return copy;
	}

	const T &operator*() const { return *node; }

	const T *operator->() const { return node; }

	bool operator==(const PoolConstIterator<T> rhs) const { return node == rhs.node; }

	bool operator!=(const PoolConstIterator<T> rhs) const { return node != rhs.node; }

private:
	const PoolNode<T> *node;
};

}

template<typename T, int partition_capacity, bool first_partition_inline> class Pool
{
	struct Empty
	{
	};

	WIN_NO_COPY_MOVE(Pool);

public:
	typedef impl::PoolIterator<T> Iterator;
	typedef impl::PoolConstIterator<T> ConstIterator;

	Pool()
	{
		static_assert(partition_capacity > 0, "Capacity must be greater than zero.");

		if constexpr (!first_partition_inline)
			first_partition_heap_ptr.reset(new impl::PoolPartition<T, partition_capacity>);
	}

	~Pool() { clear(); }

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

	template<typename... Ts> T &add(Ts &&...ts)
	{
		if (count == std::numeric_limits<int>::max())
			win::bug("win::Pool: max size");

		auto *const spot = find_spot();
		auto *const node = std::launder(new (spot) impl::PoolNode<T>(std::forward<Ts>(ts)...));

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

	void remove(T &object) { erase(object); }

	Iterator remove(Iterator it) { return erase(*it); }

private:
	Iterator erase(T &object)
	{
		auto node = static_cast<impl::PoolNode<T> *>(&object);

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
			freelist.push_back(node);

		impl::PoolNode<T> *next = node->next;

#ifndef NDEBUG
		node->next = NULL;
#endif

		node->~PoolNode();

		return Iterator(next);
	}

	impl::PoolNode<T> *find_spot()
	{
		if (!freelist.empty())
		{
			auto spot = freelist[freelist.size() - 1];
			freelist.pop_back();

			return spot;
		}

		const int partition_number = count / partition_capacity;
		int partition_index = count % partition_capacity;

		auto partition = get_first_partition();
		for (int i = 0; i < partition_number; ++i)
		{
			if (!partition->next)
				partition->next.reset(new impl::PoolPartition<T, partition_capacity>());

			partition = partition->next.get();
		}

		// The reinterpret_cast comes close to violating strict-aliasing here, but since the pointer is not dereferenced, it's ok
		return reinterpret_cast<impl::PoolNode<T> *>(&partition->storage[partition_index].item);
	}

	constexpr impl::PoolPartition<T, partition_capacity> *get_first_partition()
	{
		if constexpr (first_partition_inline)
			return &first_partition_storage;
		else
			return first_partition_heap_ptr.get();
	}

	impl::PoolNode<T> *head = NULL;
	impl::PoolNode<T> *tail = NULL;

	[[no_unique_address]]
	std::conditional_t<!first_partition_inline, std::unique_ptr<impl::PoolPartition<T, partition_capacity>>, Empty> first_partition_heap_ptr;

	[[no_unique_address]]
	std::conditional_t<first_partition_inline, impl::PoolPartition<T, partition_capacity>, Empty> first_partition_storage;

	std::vector<impl::PoolNode<T> *> freelist;

	int count = 0;
};

}
