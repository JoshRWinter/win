#ifndef WIN_EFFECT_CONTAINER_HPP
#define WIN_EFFECT_CONTAINER_HPP

#include <win/win.hpp>
#include <win/sound/soundeffect.hpp>

namespace win
{

struct EffectContainerItem
{
	int priority;
	SoundEffect *effect;
};

template <int maxlen> class EffectContainer
{
	WIN_NO_COPY_MOVE(EffectContainer);
public:
	class EffectContainerIterator
	{
		friend EffectContainer;
		WIN_NO_COPY_MOVE(EffectContainerIterator);
		explicit EffectContainerIterator(EffectContainerItem *e) : current(e) {}
	public:
		SoundEffect *operator->() { return current->effect; }
		SoundEffect *operator*() { return current->effect; }
		EffectContainerIterator operator++() { ++current; return *this; }
		EffectContainerIterator operator++(int) { EffectContainerIterator tmp = *this; ++current; return tmp; }
		bool operator==(EffectContainerIterator rhs) const { return current == rhs.current; }
		bool operator!=(EffectContainerIterator rhs) const { return current != rhs.current; }
	private:
		EffectContainerItem *current;
	};

	static_assert(maxlen > 0, "maxlen must be greater than 0");

	EffectContainer() : len(0) {}

	EffectContainerIterator begin() { return EffectContainerIterator(effects); }
	EffectContainerIterator end() { return EffectContainerIterator(NULL); }

	SoundEffect *push_back(int priority, SoundEffect *effect)
	{
		if (len == maxlen)
			win::bug("EffectContainer: too many effects");

		int spot = -1;
		for (int i = 0; i < len; ++i)
		{
			if (priority < effects[i].priority)
			{
				spot = i;
				break;
			}
		}

		if (spot == -1)
		{
			effects[len].priority = priority;
			effects[len].effect = effect;
			++len;
		}
		else
		{
			for (int i = len; i > spot; --i)
			{
				effects[i].priority = effects[i - 1].priority;
				effects[i].effect = effects[i - 1].effect;
			}

			effects[spot].priority = priority;
			effects[spot].effect = effect;
			++len;
		}

		return effect;
	}

	EffectContainerIterator erase(EffectContainerIterator it)
	{
		if (it.current >= effects + maxlen || it.current < effects)
			win::bug("EffectContainer: out of range");

		const int spot = it.current - effects;
		for (int i = spot; i < len - 1; ++i)
		{
			effects[i].effect = effects[i - 1].effect;
			effects[i].priority = effects[i - 1].priority;
		}

		--len;

		if (spot == len)
			it.current = NULL;

		return it;
	}

	void erase(SoundEffect *effect)
	{
		int spot = -1;
		for (int i = 0; i < len; ++i)
		{
			if (effects[i].effect == effect)
			{
				spot = i;
				break;
			}
		}

		if (spot == -1)
			win::bug("EffectContainer: not found");

		for (int i = spot; i < len - 1; ++i)
		{
			effects[i].priority = effects[i + 1].priority;
			effects[i].effect = effects[i + 1].effect;
		}

		--len;
	}

private:
	EffectContainerItem effects[maxlen];
	int len;
};

}

#endif
