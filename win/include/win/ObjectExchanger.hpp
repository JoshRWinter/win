#pragma once

#include <atomic>

#include <win/Win.hpp>

namespace win
{

namespace impl
{

enum class ObjectExchangerState
{
	empty,
	writer_locked,
	ready,
	reader_locked
};

// private -- don't use directly
template<typename T> struct Object : T
{
	WIN_NO_COPY_MOVE(Object);

	Object()
		: state(ObjectExchangerState::empty)
	{}

	std::atomic<ObjectExchangerState> state;
};

}

// good for multi-reader / single-writer
template<typename T, int slots> class ObjectExchanger
{
	WIN_NO_COPY_MOVE(ObjectExchanger);

	static_assert(slots > 0, "slots must be greater than 0");

public:
	ObjectExchanger() = default;

	T *writer_acquire()
	{
		for (int i = 0; i < slots; ++i)
		{
			auto expected = impl::ObjectExchangerState::empty;
			if (objects[i].state.compare_exchange_strong(expected, impl::ObjectExchangerState::writer_locked))
				return objects + i;
		}

		return NULL;
	}

	void writer_release(T *t)
	{
		auto object = (impl::Object<T> *) t;
		object->state.store(impl::ObjectExchangerState::ready);

		for (int i = 0; i < slots; ++i)
		{
			if (objects + i == object)
				continue;

			auto expected = impl::ObjectExchangerState::ready;
			objects[i].state.compare_exchange_strong(expected, impl::ObjectExchangerState::empty);
		}
	}

	T *reader_acquire()
	{
		for (int i = 0; i < slots; ++i)
		{
			auto expected = impl::ObjectExchangerState::ready;
			if (objects[i].state.compare_exchange_strong(expected, impl::ObjectExchangerState::reader_locked))
				return objects + i;
		}

		return NULL;
	}

	void reader_release(T *t)
	{
		auto object = (impl::Object<T> *) t;
		object->state.store(impl::ObjectExchangerState::empty);
	}

private:
	impl::Object<T> objects[slots];
};

}
