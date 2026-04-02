#pragma once

#include <win/Bag.hpp>
#include <win/Win.hpp>

namespace win
{

template<typename T> struct PoolBox
{
    template<typename... Ts> explicit PoolBox(Ts &&...ts)
        : inner(std::forward<Ts>(ts)...)
    {
    }

    T inner;
};

template<typename T> struct PoolNode : T
{
    template<typename... Ts> explicit PoolNode(Ts &&...ts)
        : T(std::forward<Ts>(ts)...)
    {
    }

    PoolNode<T> *prev;
    PoolNode<T> *next;
};

namespace impl
{

template<typename T> class PoolIterator
{
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

template<typename T, int partition_capacity, bool first_partition_inline, bool use_shared_bag = false> class Pool
{
    WIN_NO_COPY(Pool);

public:
    typedef impl::PoolIterator<T> Iterator;
    typedef impl::PoolConstIterator<T> ConstIterator;

    Pool() { static_assert(!use_shared_bag, "Must supply a bag object when use_shared_bag is true"); }

    explicit Pool(win::Bag<PoolNode<T>, partition_capacity, first_partition_inline> &bag)
        : bag(bag)
    {
        static_assert(use_shared_bag, "Must NOT supply a bag object when use_shared_bag is true");
    }

    Pool(Pool &&) = default;

    ~Pool() { clear(); }

    Pool &operator=(Pool &&) = default;

    Iterator begin() { return Iterator(head); }

    Iterator end() { return Iterator(NULL); }

    ConstIterator begin() const { return ConstIterator(head); }

    ConstIterator end() const { return ConstIterator(NULL); }

    int size() const { return bag.size(); }

    void clear()
    {
        Iterator it = begin();
        while (it != end())
            it = remove(it);
    }

    template<typename... Ts> T &add(Ts &&...ts)
    {
        auto node = &bag.add(std::forward<Ts>(ts)...);

        node->next = NULL;
        node->prev = tail;

        if (tail != NULL)
            tail->next = node;
        else
            head = node;

        tail = node;

        return *node;
    }

    void remove(T &object) { erase(object); }

    Iterator remove(Iterator it) { return erase(*it); }

private:
    Iterator erase(T &object)
    {
        auto node = static_cast<PoolNode<T> *>(&object);

        if (node->prev != NULL)
            node->prev->next = node->next;
        else
            head = node->next;

        if (node->next != NULL)
            node->next->prev = node->prev;
        else
            tail = node->prev;

        auto next = node->next;

        bag.remove(*node);

        return Iterator(next);
    }

    PoolNode<T> *head = NULL;
    PoolNode<T> *tail = NULL;

    // hoo boy clang format does my boy dirty here
    // clang-format off
	std::conditional_t<use_shared_bag,
					   win::Bag<PoolNode<T>, partition_capacity, first_partition_inline> &,
					   win::Bag<PoolNode<T>, partition_capacity, first_partition_inline>> bag;
    // clang-format on
};

}
