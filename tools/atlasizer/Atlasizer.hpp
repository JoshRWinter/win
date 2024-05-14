#pragma once

#include <vector>

#include <win/Win.hpp>

struct AtlasItem
{
	AtlasItem(int texture, int x, int y, int w, int h)
		: texture(texture), x(x), y(y), w(w), h(h) {}

	int texture;
	int x;
	int y;
	int w;
	int h;
	bool valid = true;
};

class Atlasizer
{
	WIN_NO_COPY_MOVE(Atlasizer);

public:
	Atlasizer() = default;

	void add(int texture, int w, int h);
	void start_drag(int x, int y);
	void continue_drag(int x, int y);
	void set_padding(int pad);
	const std::vector<AtlasItem> &get_items() const;

private:
	void check_validity();

	std::vector<AtlasItem> items;

	int padding = 0;
	bool selection_active = false;
	int grabx = 0, graby = 0;
};
