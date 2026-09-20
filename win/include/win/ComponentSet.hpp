#pragma once

#include <type_traits>

#include <win/Bag.hpp>

namespace win
{

template<typename ComponentBase> struct ComponentChunk
{
    WIN_NO_COPY_MOVE(ComponentChunk);

    constexpr static int size = 3;

    ComponentChunk()
    {
        for (auto &c : set)
            c = NULL;
    }

    ComponentBase *set[size];
    ComponentChunk *next = NULL;
};

template<typename ComponentBase, typename Component> class ComponentSetIterator
{
public:
    ComponentSetIterator(ComponentChunk<ComponentBase> *first, int index, decltype(ComponentBase::type) filter)
        : current(first)
        , index(index)
        , filter(filter)
    {
        if (current != NULL)
            seek_to_next_valid();
    }

    void operator++()
    {
        ++index;
        seek_to_next_valid();
    }

    void operator++(int)
    {
        ++index;
        seek_to_next_valid();
    }

    Component &operator*() { return (Component &)*current->set[index]; }

    Component *operator->() { return (Component *)current->set[index]; }

    bool operator!=(const ComponentSetIterator &rhs) { return current != rhs.current || index != rhs.index; }

private:
    void seek_to_next_valid()
    {
        while (true)
        {
            if (index == ComponentChunk<ComponentBase>::size)
            {
                current = current->next;
                index = 0;

                if (current == NULL)
                    return;
            }

            if (current->set[index] == NULL || current->set[index]->type != filter)
                ++index;
            else
                return;
        }
    }

    ComponentChunk<ComponentBase> *current;
    int index;
    decltype(ComponentBase::type) filter;
};

template<typename ComponentBase, typename Component, typename ComponentType> class ComponentSetQuery
{
    WIN_NO_COPY_MOVE(ComponentSetQuery);

public:
    ComponentSetQuery(ComponentChunk<ComponentBase> *first, ComponentType filter, bool &querying)
        : first(first)
        , querying(querying)
        , filter(filter)
    {
    }

    ~ComponentSetQuery() { querying = false; }

    ComponentSetIterator<ComponentBase, Component> begin() { return ComponentSetIterator<ComponentBase, Component>(first, 0, filter); }

    ComponentSetIterator<ComponentBase, Component> end() { return ComponentSetIterator<ComponentBase, Component>(NULL, 0, filter); }

private:
    ComponentChunk<ComponentBase> *first;
    bool &querying;
    ComponentType filter;
};

template<typename ComponentBase, typename Bag> class ComponentSet
{
    WIN_NO_COPY_MOVE(ComponentSet);

    static_assert(std::is_enum_v<decltype(ComponentBase::type)>, "ComponentBase must have a type parameter that is an enum");

public:
    explicit ComponentSet(Bag &bag)
        : bag(bag)
    {
    }

    ~ComponentSet()
    {
        auto chunk = first.next;
        while (chunk != NULL)
        {
            auto c = chunk->next;
            bag.remove(*chunk);
            chunk = c;
        }
    }

    int size() const { return count; }

    ComponentBase &add(ComponentBase &component)
    {
        if (querying)
            win::bug("ComponentSet add: a query is open");

        ComponentChunk<ComponentBase> *chunk = &first;
        while (true)
        {
            for (auto &c : chunk->set)
            {
                if (c == NULL)
                {
                    ++count;
                    c = &component;
                    return component;
                }
            }

            if (chunk->next == NULL)
                chunk->next = &bag.add();

            chunk = chunk->next;
        }
    }

    template<typename Component> ComponentSetQuery<ComponentBase, Component, decltype(ComponentBase::type)> get_all()
    {
        static_assert(std::is_enum_v<decltype(Component::ctype)>, "Component must have a static enum member named ctype");

        if (querying)
            win::bug("ComponentSet get_all: a query is already open");

        return ComponentSetQuery<ComponentBase, Component, decltype(ComponentBase::type)>(&first, Component::ctype, querying);
    }

    template<typename Component> Component &get()
    {
        auto q = get_all<Component>();
        auto it = q.begin();

        if (it != q.end())
            return *it;

        win::bug("ComponentSet get: no component of type " + std::to_string(Component::ctype));
    }

    template<typename Component> Component *get_optional()
    {
        auto q = get_all<Component>();
        auto it = q.begin();

        if (it != q.end())
            return &(*it);

        return NULL;
    }

    template<typename Component> Component &remove()
    {
        auto q = get_all<Component>();
        auto it = q.begin();
        if (it != q.end())
        {
            auto &c = *it;
            remove(it);
            return c;
        }

        win::bug("ComponentSet remove: no component of type " + std::to_string(Component::ctype));
    }

    template<typename Component> void remove_all()
    {
        auto q = get_all<Component>();
        for (auto it = q.begin(); it != q.end();)
            it = remove(it);
    }

    template<typename Component> ComponentSetIterator<ComponentBase, Component> remove(ComponentSetIterator<ComponentBase, Component> &it)
    {
        it.current->set[it.index] = NULL;
        --count;
        auto copy = it;
        copy.seek_to_next_valid();
        return copy;
    }

private:
    Bag &bag;
    ComponentChunk<ComponentBase> first;
    int count = 0;
    bool querying = false;
};

}
