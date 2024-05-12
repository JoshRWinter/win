#pragma once

#include <vector>

#include <win/Win.hpp>

struct AtlasItem
{
	AtlasItem(int texture, int x, int y, int w, int h)
		: texture(texture), x(x), y(y), w(w), h(h), selected(false) {}

	int texture;
	int x;
	int y;
	int w;
	int h;

	bool selected;
};

class Atlasizer
{
	WIN_NO_COPY_MOVE(Atlasizer);

public:
	Atlasizer() = default;

	void add(int texture, int w, int h);
	const std::vector<AtlasItem> &get_items() const;

private:
	std::vector<AtlasItem> items;
};
