#include <string>

#include <win/Bag.hpp>
#include <win/Win.hpp>
#define private public
#include <win/ComponentSet.hpp>
#undef private

enum ComponentType
{
    a,
    b,
    c,
    d,
    e,
};

struct ComponentBase
{
    explicit ComponentBase(ComponentType type)
        : type(type)
    {
    }

    ComponentType type;
};

struct ComponentA : public ComponentBase
{
    static constexpr ComponentType ctype = ComponentType::a;

    ComponentA()
        : ComponentBase(ComponentType::a)
    {
    }
};

struct ComponentB : public ComponentBase
{
    static constexpr ComponentType ctype = ComponentType::b;

    ComponentB()
        : ComponentBase(ComponentType::b)
    {
    }
};

struct ComponentC : public ComponentBase
{
    static constexpr ComponentType ctype = ComponentType::c;

    ComponentC()
        : ComponentBase(ComponentType::c)
    {
    }
};

struct ComponentD : public ComponentBase
{
    static constexpr ComponentType ctype = ComponentType::d;

    ComponentD()
        : ComponentBase(ComponentType::d)
    {
    }
};

struct ComponentE : public ComponentBase
{
    static constexpr ComponentType ctype = ComponentType::e;

    ComponentE()
        : ComponentBase(ComponentType::e)
    {
    }
};

#define compsetassert(exp)                                                                                                                                     \
    if (!(exp))                                                                                                                                                \
    win::bug("component set assert failed (" + std::to_string(__LINE__) + "): " #exp)

void run()
{
    win::Bag<win::ComponentChunk<ComponentBase>, 100, true> bag;
    win::ComponentSet<ComponentBase, decltype(bag)> cs(bag);

    {
        auto query = cs.get_all<ComponentA>();
        auto begin = query.begin();
        auto end = query.end();

        compsetassert(begin.current == NULL);
        compsetassert(begin.index == 0);
        compsetassert(begin.filter == ComponentType::a);

        compsetassert(end.current == NULL);
        compsetassert(end.index == 0);
        compsetassert(end.filter == ComponentType::a);

        compsetassert(!(begin != end));
    }

    ComponentA a;
    ComponentB b;
    ComponentC c;
    ComponentD d;
    ComponentE e;

    cs.add(a);

    {
        compsetassert(cs.size() == 1);

        auto query = cs.get_all<ComponentA>();
        auto begin = query.begin();
        auto end = query.end();

        compsetassert(begin.current == &cs.first);
        compsetassert(begin.index == 0);
        compsetassert(begin.filter == ComponentType::a);

        compsetassert(end.current == NULL);
        compsetassert(end.index == 0);
        compsetassert(end.filter == ComponentType::a);

        compsetassert(begin != end);

        compsetassert(&(*begin) == &a);
        compsetassert(begin->type == ComponentType::a);

        ++begin;

        compsetassert(begin.current == NULL);
        compsetassert(begin.index == 0);
        compsetassert(!(begin != end));
    }

    {
        compsetassert(cs.size() == 1);

        auto query = cs.get_all<ComponentB>();
        auto begin = query.begin();
        auto end = query.end();

        compsetassert(begin.current == NULL);
        compsetassert(begin.index == 0);
        compsetassert(begin.filter == ComponentType::b);

        compsetassert(end.current == NULL);
        compsetassert(end.index == 0);
        compsetassert(end.filter == ComponentType::b);

        compsetassert(!(begin != end));
    }

    cs.add(b);
    cs.add(b);
    cs.add(c);
    cs.add(d);
    cs.add(e);

    compsetassert(cs.size() == 6);
    compsetassert(cs.first.next != NULL);
    compsetassert(cs.first.next->next == NULL);

    {
        auto query = cs.get_all<ComponentB>();
        auto begin = query.begin();
        auto end = query.end();

        compsetassert(begin.current == &cs.first);
        compsetassert(begin.index == 1);
        compsetassert(begin.filter == ComponentType::b);

        compsetassert(end.current == NULL);
        compsetassert(end.index == 0);
        compsetassert(end.filter == ComponentType::b);

        compsetassert(begin != end);

        compsetassert(&(*begin) == &b);
        compsetassert(begin->type == ComponentType::b);

        ++begin;

        compsetassert(begin.current == &cs.first);
        compsetassert(begin.index == 2);

        compsetassert(begin != end);

        compsetassert(&(*begin) == &b);
        compsetassert(begin->type == ComponentType::b);

        ++begin;

        compsetassert(begin.current == NULL);
        compsetassert(begin.index == 0);

        compsetassert(!(begin != end));
    }

    {
        auto query = cs.get_all<ComponentE>();
        auto begin = query.begin();
        auto end = query.end();

        compsetassert(begin.current == cs.first.next);
        compsetassert(begin.index == 2);
        compsetassert(begin.filter == ComponentType::e);

        compsetassert(end.current == NULL);
        compsetassert(end.index == 0);
        compsetassert(end.filter == ComponentType::e);

        compsetassert(begin != end);

        compsetassert(&(*begin) == &e);
        compsetassert(begin->type == ComponentType::e);

        ++begin;

        compsetassert(begin.current == NULL);
        compsetassert(begin.index == 0);

        compsetassert(!(begin != end));
    }

    compsetassert(cs.get_optional<ComponentD>()->type == ComponentType::d);
    compsetassert(cs.get<ComponentD>().type == ComponentType::d);
    cs.remove<ComponentD>();
    compsetassert(cs.get_optional<ComponentD>() == NULL);
    compsetassert(cs.size() == 5);

    compsetassert(cs.get_optional<ComponentE>()->type == ComponentType::e);
    compsetassert(cs.get<ComponentE>().type == ComponentType::e);
    cs.remove<ComponentE>();
    compsetassert(cs.get_optional<ComponentE>() == NULL);
    compsetassert(cs.size() == 4);

    compsetassert(cs.get_optional<ComponentA>()->type == ComponentType::a);
    compsetassert(cs.get<ComponentA>().type == ComponentType::a);
    cs.remove<ComponentA>();
    compsetassert(cs.get_optional<ComponentA>() == NULL);
    compsetassert(cs.size() == 3);

    compsetassert(cs.get_optional<ComponentC>()->type == ComponentType::c);
    compsetassert(cs.get<ComponentC>().type == ComponentType::c);
    cs.remove<ComponentC>();
    compsetassert(cs.get_optional<ComponentC>() == NULL);
    compsetassert(cs.size() == 2);

    compsetassert(cs.get_optional<ComponentB>()->type == ComponentType::b);
    compsetassert(cs.get<ComponentB>().type == ComponentType::b);
    cs.remove_all<ComponentB>();
    compsetassert(cs.get_optional<ComponentB>() == NULL);
    compsetassert(cs.size() == 0);
}

int main()
{
    run();

    fprintf(stderr, "All tests ran successfully\n");

    return 0;
}
