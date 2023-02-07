#include <thread>

#define private public
#include <win/sound/ActiveSoundStore.hpp>
#undef private

#define assert(exp) if (!(exp)) win::bug("Assertion failed (" #exp ") on line " + std::to_string(__LINE__))

struct TestObject
{
	TestObject(int &num, const std::string &name)
		: num(num)
		, name(name)
	{
		++num;
	}
	~TestObject()
	{
		--num;
	}

	int &num;
	const std::string name;
};

static std::uint16_t get_id(std::uint32_t key) { return key >> 16; }
static std::uint16_t get_index(std::uint32_t key) { return key & 0xffff; }
static std::uint32_t get_key(std::uint16_t id, std::uint16_t index)
{
	std::uint32_t key = id;
	key <<= 16;
	key |= index;
	return key;
}

static void smoke()
{
	win::ActiveSoundStore<TestObject, 32> store;

	// add a bunch of stuff, make sure the basics work
	int num = 0;
	const std::uint32_t red = store.add(num, "red");
	assert(num == 1);
	assert(store.size() == 1);
	const std::uint32_t orange = store.add(num, "orange");
	assert(num == 2);
	assert(store.size() == 2);
	const std::uint32_t yellow = store.add(num, "yellow");
	assert(num == 3);
	assert(store.size() == 3);
	const std::uint32_t green = store.add(num, "green");
	assert(num == 4);
	assert(store.size() == 4);
	const std::uint32_t blue = store.add(num, "blue");
	assert(num == 5);
	assert(store.size() == 5);
	const std::uint32_t indigo = store.add(num, "indigo");
	assert(num == 6);
	assert(store.size() == 6);
	const std::uint32_t violet = store.add(num, "violet");
	assert(num == 7);
	assert(store.size() == 7);

	assert(get_id(red) == 0);
	assert(get_index(red) == 0);
	assert(store[red]->name == "red");

	assert(get_id(orange) == 1);
	assert(get_index(orange) == 1);
	assert(store[orange]->name == "orange");

	assert(get_id(yellow) == 2);
	assert(get_index(yellow) == 2);
	assert(store[yellow]->name == "yellow");

	assert(get_id(green) == 3);
	assert(get_index(green) == 3);
	assert(store[green]->name == "green");

	assert(get_id(blue) == 4);
	assert(get_index(blue) == 4);
	assert(store[blue]->name == "blue");

	assert(get_id(indigo) == 5);
	assert(get_index(indigo) == 5);
	assert(store[indigo]->name == "indigo");

	assert(get_id(violet) == 6);
	assert(get_index(violet) == 6);
	assert(store[violet]->name == "violet");

	assert(store.index_free_list.size() == 0);

	// try to remove something with the wrong id
	store.remove(get_key(16, 1));
	// try to get something that never existed
	store.remove(get_key(0, 31));
	// try to get something with the wrong id
	assert(store[get_key(16, 1)] == NULL);
	// try to get something that never existed
	assert(store[get_key(1, 31)] == NULL);

	// remove yellow
	store.remove(yellow);
	assert(store[yellow] == NULL);
	assert(store.index[get_index(yellow)] == NULL);
	assert(store.index_free_list.size() == 1);
	assert(store.index_free_list.at(0) == 2);
	assert(num == 6);
	assert(store.size() == 6);

	const std::uint32_t rainbow = store.add(num, "rainbow");
	assert(get_id(rainbow) == 7);
	assert(get_index(rainbow) == 2);
	assert(store[rainbow]->name == "rainbow");
	assert(store.index_free_list.size() == 0);
	assert(num == 7);
	assert(store.size() == 7);

	// try to get yellow
	assert(store[yellow] == NULL);
	// try to remove yellow
	store.remove(yellow);

	// remove everything
	store.remove(rainbow);
	assert(num == 6);
	assert(store.size() == 6);
	assert(store.index_free_list.size() == 1);
	assert(store.index_free_list.at(0) == 2);

	store.remove(red);
	assert(num == 5);
	assert(store.size() == 5);
	assert(store.index_free_list.size() == 2);
	assert(store.index_free_list.at(1) == 0);

	store.remove(indigo);
	assert(num == 4);
	assert(store.size() == 4);
	assert(store.index_free_list.size() == 3);
	assert(store.index_free_list.at(2) == 5);

	store.remove(green);
	assert(num == 3);
	assert(store.size() == 3);
	assert(store.index_free_list.size() == 4);
	assert(store.index_free_list.at(3) == 3);

	store.remove(orange);
	assert(num == 2);
	assert(store.size() == 2);
	assert(store.index_free_list.size() == 5);
	assert(store.index_free_list.at(4) == 1);

	store.remove(blue);
	assert(num == 1);
	assert(store.size() == 1);
	assert(store.index_free_list.size() == 6);
	assert(store.index_free_list.at(5) == 4);

	store.remove(violet);
	assert(num == 0);
	assert(store.size() == 0);
	assert(store.index_free_list.size() == 0);
}

static void smoke_with_iterators()
{
	win::ActiveSoundStore<TestObject, 32> store;
	int num = 0;
	store.add(num, "red");
	store.add(num, "orange");
	store.add(num, "yellow");
	store.add(num, "green");
	store.add(num, "blue");
	store.add(num, "indigo");
	store.add(num, "violet");

	win::ActiveSoundStore<TestObject, 32>::Iterator it = store.begin();

	assert(it->name == "red");
	it++;
	assert((*it).name == "orange");
	it++;
	assert((*it).name == "yellow");
	win::ActiveSoundStore<TestObject, 32>::Iterator it2 = it++;
	assert(it2->name == "yellow");
	assert((*it).name == "green");
	it++;
	assert((*it).name == "blue");
	it++;
	assert((*it).name == "indigo");
	it++;
	assert((*it).name == "violet");
	it++;

	assert(it == store.end());

	it = store.begin();
	++it; ++it;
	assert(it->name == "yellow");

	it = store.remove(it);
	assert(it->name == "green");
	assert(num == 6);

	it = store.remove(it);
	assert(it->name == "blue");
	assert(num == 5);

	it++;
	it = store.remove(it);
	assert(it->name == "violet");
	assert(num == 4);

	// check up on some state
	assert(store.size() == 4);
	assert(num == 4);
	assert(store.index_free_list.size() == 3);
	assert(store.index_free_list[0] == 2);
	assert(store.index_free_list[1] == 3);
	assert(store.index_free_list[2] == 5);
}

static void destructor()
{
	int num = 0;
	{
		win::ActiveSoundStore<TestObject, 32> store;
		store.add(num, "red");
		store.add(num, "orange");
		store.add(num, "yellow");
		store.add(num, "green");
		store.add(num, "blue");
		store.add(num, "indigo");
		store.add(num, "violet");

		assert(store.size() == 7);
		assert(num == 7);
	}

	assert(num == 0);
}

static void small_store()
{
	win::ActiveSoundStore<TestObject, 4> store;
	int num;

	const auto red = store.add(num, "red");
	const auto orange = store.add(num, "orange");
	const auto yellow = store.add(num, "yellow");
	const auto green = store.add(num, "green");
	const auto blue = store.add(num, "blue");
	const auto indigo = store.add(num, "indigo");
	const auto violet = store.add(num, "violet");

	assert(red == get_key(0, 0));
	assert(orange == get_key(1, 1));
	assert(yellow == get_key(2, 2));
	assert(green == get_key(3, 3));
	assert(blue == -1);
	assert(indigo == -1);
	assert(violet == -1);

	assert(store[red]->name == "red");
	assert(store[blue] == NULL);
}

int main()
{
	smoke();
	smoke_with_iterators();
	destructor();
	small_store();

	fprintf(stderr, "All tests ran successfully\n");
}
