#include <win/Win.hpp>

#define private public
#include <win/BlockMap.hpp>

static int successfull = 0;
#define assert(expression) do { if (!(expression)) { win::bug("Assert failed on line " + std::to_string(__LINE__)); } else { ++successfull; } } while (false)

static void print_sequence(const std::vector<int> &seq)
{
	bool first = true;
	for (auto i : seq)
	{
		if (first)
			first = false;
		else
			fprintf(stderr, ",");

		fprintf(stderr, "%d", i);
	}

	fprintf(stderr, "\n");
}

static bool sequence_equal(const std::vector<int> &a, const std::vector<int> &b)
{
	if (a.size() != b.size())
		return false;

	for (int i = 0; i < a.size(); ++i)
	{
		if (a.at(i) != b.at(i))
			return false;
	}

	return true;
}

static std::vector<int> to_list(win::BlockMap<int> &map, const win::BlockMapLocation &loc)
{
	std::vector<int> result;

	for (auto item : map.query(loc))
	{
		result.push_back(item);
	}

	return result;
}

void basic_tests()
{
	// empty case
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		auto items = to_list(map, win::BlockMapLocation(0, 0, 0, 0));

		assert(items.size() == 0);
	}

	// ----------------- miss cases

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(1.5f, 1.5f, 0.0, 0.0)); // check upper right quad
		assert(items.size() == 0);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 1.2f, 0.0, 0.0)); // check upper left quad
		assert(items.size() == 0);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(1.2f, 0.2f, 0.0, 0.0)); // check lower right quad
		assert(items.size() == 0);
	}

	// ----------------- hit cases

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 1.5f, 0.0f, 0.0f), id); // upper left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 1.2f, 0.0, 0.0)); // check upper left quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), id); // upper right quad

		auto items = to_list(map, win::BlockMapLocation(1.2f, 1.2f, 0.0, 0.0)); // check upper right quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), id); // upper right quad

		auto items = to_list(map, win::BlockMapLocation(1.2f, 1.2f, 0.0, 0.0)); // check upper right quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 0.5f, 0.0f, 0.0f), id); // lower right quad

		auto items = to_list(map, win::BlockMapLocation(1.2f, 0.2f, 0.0, 0.0)); // check lower right quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), id); // upper right quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 1.0, 1.0)); // check all quads
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 1.5f, 0.0f, 0.0f), id); // upper left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 1.0, 1.0)); // check all quads
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 0.5f, 0.0f, 0.0f), id); // lower right quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 1.0, 1.0)); // check all quads
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));
	}

	// -------------------- removal cases

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));

		map.remove(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // remove item

		auto items2 = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check all quads
		assert(items2.size() == 0);
	}
}

void move_tests()
{
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));

		map.move(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), win::BlockMapLocation(0.6f, 0.6f, 0.0f, 0.0f), id); // move to lower left quad (same)

		auto items2 = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items2.size() == 1);
		assert(sequence_equal(items2, { 0 }));
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), id); // lower left quad

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items.size() == 1);
		assert(sequence_equal(items, { 0 }));

		map.move(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), id); // move to upper right quad

		auto items2 = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items2.size() == 0);

		auto items3 = to_list(map, win::BlockMapLocation(1.2f, 1.2f, 0.0, 0.0)); // check upper right quad
		assert(items3.size() == 1);
		assert(sequence_equal(items3, {0}));
	}
}

void vacuum_tests()
{
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		// ASSUMING THE VACCUM THRESHOLD IS 10
		const int vacuum_threshold = 9;
		std::vector<int> inputs;
		for (int i = 0; i < vacuum_threshold; ++i)
		{
			inputs.push_back(i);
			map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), inputs[i]); // lower left quad
			map.remove(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), inputs[i]);
		}

		auto items = to_list(map, win::BlockMapLocation(0.2f, 0.2f, 0.0, 0.0)); // check lower left quad
		assert(items.size() == 0);

		int ghosts = 0;
		for (auto &block : map.map)
		{
			for (auto &item : block.items)
				if (item == NULL)
					++ghosts;
		}

		assert(ghosts == 9);

		std::vector<int> inputs2;
		for (int i = 0; i < vacuum_threshold; ++i)
		{
			inputs2.push_back(i);
			map.add(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), inputs[i]); // upper right quad
			map.remove(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), inputs[i]);
		}

		auto items2 = to_list(map, win::BlockMapLocation(1.2f, 1.2f, 0.0, 0.0)); // check upper left quad
		assert(items2.size() == 0);

		ghosts = 0;
		for (auto &block : map.map)
		{
			for (auto &item : block.items)
				if (item == NULL)
					++ghosts;
		}

		assert(ghosts == 18);

		// add and remove 1 more item from both quadrants
		int onemore = 69, onemore2 = 420;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), onemore);
		map.remove(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 0.0f), onemore);
		map.add(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), onemore2);
		map.remove(win::BlockMapLocation(1.5f, 1.5f, 0.0f, 0.0f), onemore2);

		// trigger a vacuum by iterating
		to_list(map, win::BlockMapLocation(0.0f, 0.0f, 0.0f, 0.0f));

		ghosts = 0;
		for (auto &block : map.map)
		{
			for (auto &item : block.items)
				if (item == NULL)
					++ghosts;
		}

		assert(ghosts == 0);
	}
}

void dedupe_tests()
{
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 1.0f, 1.0f), id); // all quads

		auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 1.0)); // check all quads
		assert(all_items.size() == 1);

		auto lower_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 0.0)); // check lower quads
		assert(lower_items.size() == 1);

		auto upper_items = to_list(map, win::BlockMapLocation(0.5f, 1.5f, 1.0, 0.0)); // check upper quads
		assert(upper_items.size() == 1);

		auto right_items = to_list(map, win::BlockMapLocation(1.5f, 0.5f, 0.0, 1.0)); // check right quads
		assert(right_items.size() == 1);

		auto left_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 0.0, 1.0)); // check left quads
		assert(left_items.size() == 1);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 1.0f, 0.0f), id); // lower quads

		auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 1.0)); // check all quads
		assert(all_items.size() == 1);

		auto lower_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 0.0)); // check lower quads
		assert(lower_items.size() == 1);

		auto upper_items = to_list(map, win::BlockMapLocation(0.5f, 1.5f, 1.0, 0.0)); // check upper quads
		assert(upper_items.size() == 0);

		auto right_items = to_list(map, win::BlockMapLocation(1.5f, 0.5f, 0.0, 1.0)); // check right quads
		assert(right_items.size() == 1);

		auto left_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 0.0, 1.0)); // check left quads
		assert(left_items.size() == 1);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 1.5f, 1.0f, 0.0f), id); // upper quads

		auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 1.0)); // check all quads
		assert(all_items.size() == 1);

		auto lower_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 0.0)); // check lower quads
		assert(lower_items.size() == 0);

		auto upper_items = to_list(map, win::BlockMapLocation(0.5f, 1.5f, 1.0, 0.0)); // check upper quads
		assert(upper_items.size() == 1);

		auto right_items = to_list(map, win::BlockMapLocation(1.5f, 0.5f, 0.0, 1.0)); // check right quads
		assert(right_items.size() == 1);

		auto left_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 0.0, 1.0)); // check left quads
		assert(left_items.size() == 1);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(1.5f, 0.5f, 0.0f, 1.0f), id); // right quads

		auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 1.0)); // check all quads
		assert(all_items.size() == 1);

		auto lower_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 0.0)); // check lower quads
		assert(lower_items.size() == 1);

		auto upper_items = to_list(map, win::BlockMapLocation(0.5f, 1.5f, 1.0, 0.0)); // check upper quads
		assert(upper_items.size() == 1);

		auto right_items = to_list(map, win::BlockMapLocation(1.5f, 0.5f, 0.0, 1.0)); // check right quads
		assert(right_items.size() == 1);

		auto left_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 0.0, 1.0)); // check left quads
		assert(left_items.size() == 0);
	}

	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 0.0f, 1.0f), id); // left quads

		auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 1.0)); // check all quads
		assert(all_items.size() == 1);

		auto lower_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 1.0, 0.0)); // check lower quads
		assert(lower_items.size() == 1);

		auto upper_items = to_list(map, win::BlockMapLocation(0.5f, 1.5f, 1.0, 0.0)); // check upper quads
		assert(upper_items.size() == 1);

		auto right_items = to_list(map, win::BlockMapLocation(1.5f, 0.5f, 0.0, 1.0)); // check right quads
		assert(right_items.size() == 0);

		auto left_items = to_list(map, win::BlockMapLocation(0.5f, 0.5f, 0.0, 1.0)); // check left quads
		assert(left_items.size() == 1);
	}
}

void remove_tests()
{
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.5f, 0.5f, 1.0f, 1.0f), id); // straddles all 4 blocks halfway

		{
			// test the lower left block
			const auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5, 0.0f, 0.0f));
			assert(all_items.size() == 1);
		}

		// remove the item
		map.remove(win::BlockMapLocation(0.5f, 0.5f, 1.0f, 1.0f), id);

		{
			// test the lower left block
			const auto all_items = to_list(map, win::BlockMapLocation(0.5f, 0.5, 0.0f, 0.0f));
			assert(all_items.size() == 0);
		}

		{
			// test the lower right block
			const auto all_items = to_list(map, win::BlockMapLocation(1.5f, 0.5, 0.0f, 0.0f));
			assert(all_items.size() == 0);
		}

		{
			// test the upper left block
			const auto all_items = to_list(map, win::BlockMapLocation(0.5f, 1.5, 0.0f, 0.0f));
			assert(all_items.size() == 0);
		}

		{
			// test the upper right block
			const auto all_items = to_list(map, win::BlockMapLocation(1.5f, 1.5, 0.0f, 0.0f));
			assert(all_items.size() == 0);
		}
	}
}

void out_of_bounds_tests()
{
	// add items wayy out of bounds, and check that they are unreachable by all query operations
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(-10.0f, -10.0f, 2.0f, 2.0f), id); // beyond lower left corner
		map.add(win::BlockMapLocation(10.0f, -10.0f, 2.0f, 2.0f), id); // beyond lower right corner
		map.add(win::BlockMapLocation(10.0f, 10.0f, 2.0f, 2.0f), id); // beyond upper right corner
		map.add(win::BlockMapLocation(-10.0f, 10.0f, 2.0f, 2.0f), id); // beyond upper left corner

		map.add(win::BlockMapLocation(-10.0f, 0.0f, 2.0f, 2.0f), id); // beyond the left edge
		map.add(win::BlockMapLocation(10.0f, 0.0f, 2.0f, 2.0f), id); // beyond the right edge
		map.add(win::BlockMapLocation(0.0f, 10.0f, 2.0f, 2.0f), id); // beyond the top edge
		map.add(win::BlockMapLocation(0.0f, -10.0f, 2.0f, 2.0f), id); // beyond the bottom edge

		// query the whole range
		const auto all_items = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f));
		assert(all_items.size() == 0);

		// remove some of the items, ensure no crash?
		map.remove(win::BlockMapLocation(-10.0f, -10.0f, 2.0f, 2.0f), id);
		map.remove(win::BlockMapLocation(10.0f, 10.0f, 2.0f, 2.0f), id);
	}

	// add a normal item, then query wayy out of bounds for it
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 0;
		map.add(win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f), id); // covers the whole map

		// query beyond lower left corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(-10.0f, -10.0f, 2.0f, 2.0f));
			assert(all_items.size() == 0);
		}

		// query beyond lower right corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(10.0f, -10.0f, 2.0f, 2.0f));
			assert(all_items.size() == 0);
		}

		// query beyond upper right corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(10.0f, 10.0f, 2.0f, 2.0f));
			assert(all_items.size() == 0);
		}

		// query beyond upper left corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(-10.0f, 10.0f, 2.0f, 2.0f));
			assert(all_items.size() == 0);
		}
	}

	// add a normal item, query partially out of bounds for it
	{
		win::BlockMap<int> map;
		map.reset(1, 0, 2, 0, 2);

		int id = 99;
		map.add(win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f), id); // covers the whole map

		// query overlapping lower left corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(-10.0f, -10.0f, 11.0f, 11.0f));
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);
		}

		// query overlapping lower right corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(1.0f, -10.0f, 11.0f, 11.0f));
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);
		}

		// query overlapping upper right corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(1.0f, 1.0f, 10.0f, 10.0f));
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);
		}

		// query overlapping upper left corner
		{
			const auto all_items = to_list(map, win::BlockMapLocation(-10.0f, 1.0f, 11.0f, 11.0f));
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);
		}
	}

	// add a items that are partially out of bounds, then remove them
	{
		{
			win::BlockMap<int> map;
			map.reset(1, 0, 2, 0, 2);

			int id = 99;
			map.add(win::BlockMapLocation(-1.0f, -1.0f, 2.0f, 2.0f), id); // overlaps the lower left corner

			const auto all_items = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);

			// remove the item
			map.remove(win::BlockMapLocation(-1.0f, -1.0f, 2.0f, 2.0f), id); // overlaps the lower left corner

			const auto all_items2 = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items2.empty());
		}

		{
			win::BlockMap<int> map;
			map.reset(1, 0, 2, 0, 2);

			int id = 99;
			map.add(win::BlockMapLocation(1.0f, -1.0f, 2.0f, 2.0f), id); // overlaps the lower right corner

			const auto all_items = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);

			// remove the item
			map.remove(win::BlockMapLocation(1.0f, -1.0f, 2.0f, 2.0f), id); // overlaps the lower right corner

			const auto all_items2 = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items2.empty());
		}

		{
			win::BlockMap<int> map;
			map.reset(1, 0, 2, 0, 2);

			int id = 99;
			map.add(win::BlockMapLocation(1.0f, 1.0f, 2.0f, 2.0f), id); // overlaps the upper right corner

			const auto all_items = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);

			// remove the item
			map.remove(win::BlockMapLocation(1.0f, 1.0f, 2.0f, 2.0f), id); // overlaps the upper right corner

			const auto all_items2 = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items2.empty());
		}

		{
			win::BlockMap<int> map;
			map.reset(1, 0, 2, 0, 2);

			int id = 99;
			map.add(win::BlockMapLocation(-1.0f, 1.0f, 2.0f, 2.0f), id); // overlaps the upper left corner

			const auto all_items = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items.size() == 1);
			assert(all_items.at(0) == 99);

			// remove the item
			map.remove(win::BlockMapLocation(-1.0f, 1.0f, 2.0f, 2.0f), id); // overlaps the upper left corner

			const auto all_items2 = to_list(map, win::BlockMapLocation(0.0f, 0.0f, 2.0f, 2.0f)); // covers the whole map
			assert(all_items2.empty());
		}
	}
}

int main()
{
	basic_tests();
	move_tests();
	vacuum_tests();
	dedupe_tests();
	remove_tests();
	out_of_bounds_tests();

	fprintf(stderr, "all %d tests ran successfully\n", successfull);
}
