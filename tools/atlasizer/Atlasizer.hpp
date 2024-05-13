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
};

class Atlasizer
{
	WIN_NO_COPY_MOVE(Atlasizer);

public:
	Atlasizer() = default;

	void add(int texture, int w, int h);
	void start_drag(int x, int y);
	void continue_drag(int x, int y);
	const std::vector<AtlasItem> &get_items() const;

private:
	std::vector<AtlasItem> items;

	bool selection_active = false;
	int grabx = 0, graby = 0;
};
